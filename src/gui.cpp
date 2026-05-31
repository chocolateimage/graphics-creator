#include "gui.hpp"
#include "fontcombobox.hpp"
#include "line.hpp"
#include "lua.hpp"
#include "lua_state.hpp"
#include "render.hpp"
#include "variant.hpp"
#include <KActionCollection>
#include <KActionMenu>
#include <KColorButton>
#include <KIconTheme>
#include <KMessageBox>
#include <KMessageWidget>
#include <KStyleManager>
#include <KTextEditor/View>
#include <QApplication>
#include <QCloseEvent>
#include <QElapsedTimer>
#include <QFrame>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QProgressDialog>
#include <QScrollArea>
#include <QSlider>
#include <QSpacerItem>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>

void FramePreviewThread::run() {
    RenderThread renderThread;
    renderThread.init();

    bool hasError = false;

    while (true) {
        window->taskMutex.lock();
        if (window->isClosing) {
            window->taskMutex.unlock();
            renderThread.close();
            return;
        }

        if (window->tasks.isEmpty()) {
            window->taskMutex.unlock();
            QThread::usleep(1000);
            continue;
        }

        FramePreviewTask task = window->tasks.dequeue();
        window->taskMutex.unlock();
        Video *video = task.frame->video;

        if (luaDirty) {
            hasError = false;
            luaDirty = false;
            window->latestLuaMutex.lock();
            hasError = !renderThread.loadLua(window->latestLua);
            window->latestLuaMutex.unlock();
            if (hasError) {
                emit errored(QString::fromStdString(renderThread.lastError));
            }
        }

        if (optionsDirty) {
            optionsDirty = false;
            window->scriptOptionsMutex.lock();
            renderThread.updateOptions(window->scriptOptions);
            window->scriptOptionsMutex.unlock();
        }

        if (!hasError) {
            hasError = !renderThread.drawImage(
                video, task.frame->frame, task.renderX, task.renderY,
                task.renderWidth, task.renderHeight);
            if (hasError) {
                emit errored(QString::fromStdString(renderThread.lastError));
            }
        }

        emit taskDone(std::move(task));
    }
}

MainWindow::MainWindow() : QMainWindow() {
    QElapsedTimer measure;
    measure.start();
    this->setCursor(Qt::WaitCursor);
    this->resize(1200, 700);
    QTimer::singleShot(4, this, &MainWindow::loadLate);
    qDebug() << "init took" << measure.elapsed() << "ms";
}

void MainWindow::loadLate() {
    QElapsedTimer measure;
    measure.start();

    // auto menuBar = new QMenuBar(this);
    // menuBar->addMenu("File")->addAction("Quit");
    // setMenuBar(menuBar);

    auto toolbar = addToolBar("Toolbar");
    toolbar->setMovable(false);
    QWidget *spacing = new QWidget(toolbar);
    spacing->setSizePolicy(QSizePolicy::Policy::MinimumExpanding,
                           QSizePolicy::Policy::Fixed);
    toolbar->addWidget(spacing);
    newAction = toolbar->addAction(QIcon::fromTheme("list-add"), "New");
    editAction = toolbar->addAction(QIcon::fromTheme("document-edit"), "Edit");
    renderAction =
        toolbar->addAction(QIcon::fromTheme("media-record"), "Render");

    connect(newAction, &QAction::triggered, this, [this]() {
        if (stackedWidget->currentIndex() != 0) {
            if (KMessageBox::warningTwoActions(
                    this,
                    "Changing to the \"New\" tab will delete your current "
                    "graphic. "
                    "Discard your changes?",
                    "Discard changes?", KStandardGuiItem::discard(),
                    KStandardGuiItem::cancel()) != KMessageBox::PrimaryAction) {
                updateTabs();
                return;
            }
        }
        stackedWidget->setCurrentIndex(0);
        updateTabs();
    });
    connect(editAction, &QAction::triggered, this, [this]() {
        stackedWidget->setCurrentIndex(1);
        updateTabs();
    });
    connect(renderAction, &QAction::triggered, this, [this]() {
        stackedWidget->setCurrentIndex(2);
        updateTabs();
    });

    newAction->setCheckable(true);
    editAction->setCheckable(true);
    renderAction->setCheckable(true);

    spacing = new QWidget(toolbar);
    spacing->setSizePolicy(QSizePolicy::Policy::MinimumExpanding,
                           QSizePolicy::Policy::Fixed);
    toolbar->addWidget(spacing);

    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // No custom context menu set, but it still disables the right click
    toolbar->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    stackedWidget->addWidget(new QLabel("new", stackedWidget));

    auto editCentralWidget = new QWidget(stackedWidget);
    stackedWidget->addWidget(editCentralWidget);
    auto topLayout = new QVBoxLayout(editCentralWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);
    auto splitter = new QSplitter(this);
    topLayout->addWidget(splitter, 1);

    auto videoContentWidget = new QWidget();
    auto videoContentLayout = new QVBoxLayout(videoContentWidget);
    splitter->addWidget(videoContentWidget);

    auto statusBar = new QStatusBar(this);
    statusText = new QLabel(statusBar);
    statusBar->addPermanentWidget(statusText);
    setStatusBar(statusBar);

    videoControlButton = new QToolButton(this);
    connect(videoControlButton, &QToolButton::clicked, this,
            &MainWindow::toggleTimer);

    loopButton = new QToolButton(this);
    loopButton->setCheckable(true);
    loopButton->setChecked(true);
    connect(loopButton, &QToolButton::toggled, this,
            &MainWindow::updateButtons);

    previewWidget = new ImageViewer(this);
    connect(previewWidget, &ImageViewer::pixelPicked, this,
            &MainWindow::pixelPicked);
    previewWidget->setMinimumSize(10, 10);
    videoContentLayout->addWidget(previewWidget);

    auto bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    videoContentLayout->addLayout(bottomLayout);

    // TODO: a slider or something to control the time
    timeInput = new QDoubleSpinBox(this);
    durationInput = new QDoubleSpinBox(this);
    timeInput->setFixedWidth(100);
    timeInput->setSuffix(" s");
    timeInput->setSingleStep(0.5);
    durationInput->setFixedWidth(100);
    durationInput->setMaximum(INFINITY);
    durationInput->setSuffix(" s");
    connect(timeInput, &QDoubleSpinBox::valueChanged, this,
            &MainWindow::updateTimeInput);
    connect(durationInput, &QDoubleSpinBox::valueChanged, this,
            &MainWindow::updateDurationInput);
    bottomLayout->addWidget(videoControlButton);
    bottomLayout->addWidget(new QLabel("Time:", this));
    bottomLayout->addWidget(timeInput);
    bottomLayout->addWidget(new QLabel("Duration:", this));
    bottomLayout->addWidget(durationInput);
    bottomLayout->addWidget(loopButton);
    bottomLayout->addStretch();

    // TODO: dynamic
    video = new Video();
    video->frameRate = 60;
    video->width = 1920;
    video->height = 1080;
    video->duration = 5;

    auto rightSideWidget = new QWidget();
    auto rightSideLayout = new QVBoxLayout(rightSideWidget);
    splitter->addWidget(rightSideWidget);

    auto scrollArea = new QScrollArea(this);
    optionsWidget = new QWidget();
    optionsLayout = new QFormLayout(optionsWidget);
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    scrollArea->setWidget(optionsWidget);
    scrollArea->setWidgetResizable(true);
    rightSideLayout->addWidget(scrollArea, 1);

    auto editor = KTextEditor::Editor::instance();
    textDocument = editor->createDocument(this);
    textDocument->setMode("Lua");
    textDocument->setConfigValue("indent-pasted-text", "false");
    textDocument->setText(R"(function options()
    return {
        {
            id = "font",
            type = "font",
        },
        {
            id = "text",
            type = "string",
            default = "Hello world",
        },
        {
            id = "fontSize",
            type = "int",
            slider = true,
            default = 128,
            min = 0,
            max = 1000,
        },
        {
            id = "testPoint",
            type = "vector2dint",
            label = "Text position",
            default = {x = 100, y = 500},
        },
        {
            id = "borderColor",
            type = "color",
            default = {r = 255, g = 255, b = 0, a = 255},
        },
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)
    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 255
            local alpha = 255

            -- put your draw code here!

            local dist = distance(x, y, testPoint.x, testPoint.y)

            red = (1 - saturate(dist / 100)) * 255

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
)");
    textView = textDocument->createView(this);
#ifdef Q_OS_WIN
    textView->setConfigValue("font", "Consolas");
#endif
    textView->setConfigValue("theme", "VSCodium Dark");
    textView->setStatusBarEnabled(false);
    textView->setContextMenu(
        textView->defaultContextMenu()); // what else should I use??
    connect(textDocument, &KTextEditor::Document::textChanged, this,
            &MainWindow::scriptUpdated);
    rightSideLayout->addWidget(textView, 1);

    errorMessage = new KMessageWidget(this);
    errorMessage->setCloseButtonVisible(false);
    errorMessage->setVisible(false);
    errorMessage->setMessageType(KMessageWidget::MessageType::Error);
    rightSideLayout->addWidget(errorMessage);

    splitter->setSizes({500, 400});

    stackedWidget->addWidget(new QLabel("render", stackedWidget));

    // TODO: use QDateTime::currentMSecsSinceEpoch() or QElapsedTimer for better
    // precision instead of relying on microsecond timers
    timer = new QChronoTimer(this);
    timer->setInterval(
        std::chrono::microseconds((int)((1000.f * 1000.f) / video->frameRate)));
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QChronoTimer::timeout, this, &MainWindow::generate);

    durationInput->setValue(video->duration);
    updateButtons();

    stackedWidget->setCurrentIndex(1);
    updateTabs();

    // TODO: allow user to set their own thread count
    for (int i = 0; i < std::max(1, QThread::idealThreadCount() - 2); i++) {
        createThread();
    }

    scriptUpdated();
    updateStatus();
    this->unsetCursor();
    qDebug() << "late init took" << measure.elapsed() << "ms";
}

void MainWindow::updateTabs() {
    int currentIndex = stackedWidget->currentIndex();
    newAction->blockSignals(true);
    editAction->blockSignals(true);
    renderAction->blockSignals(true);

    newAction->setChecked(currentIndex == 0);
    editAction->setChecked(currentIndex == 1);
    renderAction->setChecked(currentIndex == 2);

    renderAction->setDisabled(currentIndex == 0);
    editAction->setDisabled(currentIndex == 0);

    newAction->blockSignals(false);
    editAction->blockSignals(false);
    renderAction->blockSignals(false);
}

void MainWindow::updateTimeInput(double value) {
    frameIndex = value * video->frameRate;
    updateStatus();
}

void MainWindow::updateDurationInput(double value) {
    video->duration = value;
    timeInput->setMaximum(value);
}

void MainWindow::toggleTimer() {
    if (timer->isActive()) {
        timer->stop();
    } else {
        if (frameIndex >= video->duration * video->frameRate) {
            frameIndex = 0;
        }
        timer->start();
    }
    updateButtons();
}

void MainWindow::updateButtons() {
    if (timer->isActive()) {
        videoControlButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        videoControlButton->setToolTip("Pause");
    } else {
        videoControlButton->setIcon(QIcon::fromTheme("media-playback-start"));
        videoControlButton->setToolTip("Play");
    }

    if (loopButton->isChecked()) {
        loopButton->setIcon(QIcon::fromTheme("media-repeat-all"));
        loopButton->setToolTip("Loop (on)");
    } else {
        loopButton->setIcon(QIcon::fromTheme("media-repeat-none"));
        loopButton->setToolTip("Loop (off)");
    }
}

bool MainWindow::addOptionFromLua(lua_State *L) {
    if (!lua_isnumber(L, -2) || !lua_istable(L, -1)) {
        updateError("options: key must be number. value must be table");
        return false;
    }

    lua_getfield(L, -1, "id");
    auto _optionId = lua_tostring(L, -1);
    lua_pop(L, 1);

    if (_optionId == nullptr || strlen(_optionId) == 0) {
        updateError("options: missing id");
        return false;
    }

    const std::string optionId = _optionId;

    addedScriptOptions.append(optionId);

    lua_getfield(L, -1, "type");
    auto _optionType = lua_tostring(L, -1);
    lua_pop(L, 1);

    if (_optionType == nullptr || strlen(_optionType) == 0) {
        updateError("options: missing type");
        return false;
    }

    auto optionType = Variant::typeFromString(std::string(_optionType));

    if ((int)optionType == -1) {
        updateError("options: invalid type");
        return false;
    }

    lua_getfield(L, -1, "label");
    QString optionLabel;
    auto userOptionLabel = lua_tostring(L, -1);
    lua_pop(L, 1);

    if (userOptionLabel == nullptr || strlen(userOptionLabel) == 0) {
        QString word;
        int len = optionId.size();
        for (int i = 0; i < len; i++) {
            char character = optionId[i];
            bool newWord = std::isupper(character);
            if (newWord) {
                optionLabel += " " + word;
                word = "";
            }

            if (i == 0) {
                character = std::toupper(character);
            }

            word += character;
        }
        optionLabel += " " + word;
        optionLabel = optionLabel.trimmed();
    } else {
        optionLabel = userOptionLabel;
    }

    double min = INT32_MIN;
    double max = INT32_MAX;
    lua_getfield(L, -1, "min");
    if (lua_isnumber(L, -1)) {
        min = lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
    lua_getfield(L, -1, "max");
    if (lua_isnumber(L, -1)) {
        max = lua_tonumber(L, -1);
    }
    lua_pop(L, 1);

    QWidget *widget{nullptr};

    auto variantIt = scriptOptions.find(optionId);
    Variant defaultVariant = Variant::getDefault(optionType);
    lua_getfield(L, -1, "default");
    if (!lua_isnil(L, -1)) {
        defaultVariant = Variant::getFromLua(optionType, L, -1);
    }
    lua_pop(L, 1);
    Variant variant =
        variantIt == scriptOptions.end() ? defaultVariant : variantIt->second;
    bool shouldUpdateInstantly = variantIt == scriptOptions.end();

    if (variant.type() != optionType) {
        qInfo() << "Variant is not the same (expected:" << optionType
                << " actual:" << variant.type() << "). Changing to default.";
        variant = defaultVariant;
        shouldUpdateInstantly = true;
    }

    if (shouldUpdateInstantly) {
        // not great.. but it works (changing defaults would break)
        scriptOptions.erase(optionId);
        scriptOptions.emplace(optionId, variant);
    }

    if (optionType == VariantTypeEnum::None) {
        auto line = new HorizontalLine(optionsWidget);
        line->setSizePolicy(QSizePolicy::Policy::Expanding,
                            QSizePolicy::Policy::Minimum);
        widget = line;
    } else if (optionType == VariantTypeEnum::String) {
        auto input = new QLineEdit(optionsWidget);
        input->setText(QString::fromStdString(variant.get<std::string>()));
        connect(input, &QLineEdit::textEdited, this,
                [this, optionId](const QString &text) {
                    updateOption(optionId, Variant(text.toStdString()));
                });
        widget = input;
    } else if (optionType == VariantTypeEnum::Int) {
        lua_getfield(L, -1, "slider");
        bool hasSlider = lua_toboolean(L, -1);
        lua_pop(L, 1);

        if (hasSlider) {
            auto input = new QSlider(optionsWidget);
            input->setOrientation(Qt::Horizontal);
            input->setMinimum(min);
            input->setMaximum(max);
            input->setValue(variant.get<int>());
            connect(input, &QSlider::valueChanged, this,
                    [this, optionId](int value) {
                        updateOption(optionId, Variant(value));
                    });
            widget = input;
        } else {
            auto input = new QSpinBox(optionsWidget);
            input->setMinimum(min);
            input->setMaximum(max);
            input->setValue(variant.get<int>());
            connect(input, &QSpinBox::valueChanged, this,
                    [this, optionId](int value) {
                        updateOption(optionId, Variant(value));
                    });
            widget = input;
        }
    } else if (optionType == VariantTypeEnum::Double) {
        auto input = new QDoubleSpinBox(optionsWidget);
        input->setMinimum(min);
        input->setMaximum(max);
        input->setValue(variant.get<double>());
        connect(input, &QDoubleSpinBox::valueChanged, this,
                [this, optionId](double value) {
                    updateOption(optionId, Variant(value));
                });
        widget = input;
    } else if (optionType == VariantTypeEnum::Vector2DInt) {
        widget = new QWidget(optionsWidget);
        auto layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        auto value = variant.get<Vector2DInt>();

        auto inputX = new QSpinBox(optionsWidget);
        inputX->setMinimum(min);
        inputX->setMaximum(max);
        inputX->setValue(value.x);

        auto inputY = new QSpinBox(optionsWidget);
        inputY->setMinimum(min);
        inputY->setMaximum(max);
        inputY->setValue(value.y);

        auto pickButton = new QToolButton(optionsWidget);
        pickButton->setToolTip("Pick position from preview");
        pickButton->setIcon(QIcon::fromTheme("select"));

        layout->addWidget(inputX);
        layout->addWidget(inputY);
        layout->addWidget(pickButton);

        connect(inputX, &QSpinBox::valueChanged, this,
                [this, optionId, inputY](int value) {
                    updateOption(optionId, Variant((Vector2DInt){
                                               value, inputY->value()}));
                });
        connect(inputY, &QSpinBox::valueChanged, this,
                [this, optionId, inputX](int value) {
                    updateOption(optionId, Variant((Vector2DInt){
                                               inputX->value(), value}));
                });
        connect(pickButton, &QToolButton::clicked, this,
                [this, optionId, optionLabel]() {
                    previewWidget->beginPicking(
                        QString::fromStdString(optionId),
                        "Pick \"" + optionLabel + "\"");
                });
    } else if (optionType == VariantTypeEnum::Color) {
        auto colorButton = new KColorButton(optionsWidget);
        colorButton->setAlphaChannelEnabled(true);
        auto value = variant.get<Color>();
        colorButton->setColor(QColor(value.r, value.g, value.b, value.a));
        connect(colorButton, &KColorButton::changed, this,
                [this, optionId](const QColor &newColor) {
                    updateOption(
                        optionId,
                        Variant((Color){newColor.red(), newColor.green(),
                                        newColor.blue(), newColor.alpha()}));
                });
        widget = colorButton;
    } else if (optionType == VariantTypeEnum::Font) {
        auto fontWidget = new FontComboBox(optionsWidget);
        fontWidget->setSizePolicy(QSizePolicy::Policy::Expanding,
                                  QSizePolicy::Policy::Fixed);
        auto value = variant.get<Font>();
        if (!value.path.empty()) {
            fontWidget->setFontValue(value);
        }
        connect(fontWidget, &FontComboBox::currentTextChanged, this,
                [this, optionId, fontWidget]() {
                    updateOption(optionId, Variant(fontWidget->fontValue()));
                });

        widget = fontWidget;
    }

    if (!widget) {
        widget = new QLabel("<i>no widget</i>", optionsWidget);
    }

    optionsLayout->addRow(optionLabel + ":", widget);

    return true;
}

void MainWindow::pixelPicked(QString id, QPoint position) {
    updateOption(id.toStdString(),
                 Variant((Vector2DInt{position.x(), position.y()})));
    recreateOptions();
}

void MainWindow::updateOption(const std::string &optionId, Variant variant) {
    scriptOptionsMutex.lock();
    scriptOptions.erase(optionId);
    scriptOptions.emplace(optionId, std::move(variant));
    scriptOptionsMutex.unlock();
    optionsUpdated();
}

void MainWindow::recreateOptions() {
    while (optionsLayout->rowCount() > 0) {
        optionsLayout->removeRow(0);
    }

    scriptOptionsMutex.lock();
    lua_State *L = createLuaState();
    addedScriptOptions.clear();
    bool shouldRemove = false;
    if (luaL_dostring(L, latestLua.c_str()) == LUA_OK) {
        lua_getglobal(L, "options");
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
                if (lua_istable(L, 1)) {
                    lua_pushnil(L);
                    shouldRemove = true;
                    while (lua_next(L, 1) != 0) {
                        if (!addOptionFromLua(L)) {
                            shouldRemove = false;
                        }
                        lua_pop(L, 1);
                    }
                } else {
                    updateError("options: must be table");
                }
            } else {
                auto err = lua_tostring(L, -1);
                updateError(err);
                lua_pop(L, 1);
            }
        }
    } else {
        auto err = lua_tostring(L, -1);
        updateError(err);
        lua_pop(L, 1);
    }
    if (shouldRemove) {
        for (auto it = scriptOptions.begin(); it != scriptOptions.end();) {
            if (!addedScriptOptions.contains(it->first)) {
                it = scriptOptions.erase(it);
            } else {
                it++;
            }
        }
    }
    lua_close(L);
    scriptOptionsMutex.unlock();
}

void MainWindow::scriptUpdated() {
    updateError("");

    latestLuaMutex.lock();
    latestLua = textDocument->text().toStdString();
    latestLuaMutex.unlock();

    recreateOptions();

    // mark dirty on threads
    for (auto thread : threads) {
        thread->luaDirty = true;
        thread->optionsDirty = true;
    }
}

void MainWindow::optionsUpdated() {
    for (auto thread : threads) {
        thread->optionsDirty = true;
    }
}

void MainWindow::updateError(const QString &error) {
    if (error == lastSetError)
        return;
    lastSetError = error;
    if (error.isEmpty()) {
        errorMessage->animatedHide();
    } else {
        errorMessage->animatedShow();
        errorMessage->setText(error);
    }
}

AVFrame *MainWindow::allocateFrame() {
    AVFrame *frame = video->allocateFrame();
    frame->format = AV_PIX_FMT_BGRA;
    av_frame_get_buffer(frame, 0);
    av_frame_make_writable(frame);
    memset(frame->data[0], 0, frame->width * frame->height * 4);
    return frame;
}

void MainWindow::createThread() {
    QPointer<FramePreviewThread> thread = new FramePreviewThread(this);
    thread->window = this;

    threads.append(thread);
    thread->start();

    connect(thread, &FramePreviewThread::taskDone, this, &MainWindow::taskDone);
    connect(thread, &FramePreviewThread::errored, this,
            [this](QString error) { updateError(error); });

    connect(thread, &FramePreviewThread::finished, thread, [this, thread]() {
        threads.removeOne(thread);
        thread->deleteLater();
        if (isClosing && closingDialog != nullptr) {
            closingDialog->setValue(closingDialog->value() + 1);
            if (threads.isEmpty()) {
                forceClosing = true;
                closingDialog->close();
                this->close();
            }
        }
    });
}

void MainWindow::generate() {
    if (isClosing) {
        qInfo() << "Can't generate frame because is closing";
        return;
    }
    if (isGenerating) {
        // TODO: temp. some better way to do this!!
        tooSlow = true;
        frameIndex++;
        updateStatus();
        return;
    }
    bool shouldLoop = loopButton->isChecked();
    bool atEnd = frameIndex >= video->duration * video->frameRate;
    if (atEnd) {
        if (shouldLoop) {
            frameIndex = 0;
        } else {
            toggleTimer();
            frameIndex = video->duration * video->frameRate;
        }
    }
    tooSlow = false;
    PreviewFrame *frame = new PreviewFrame();
    frame->frame = allocateFrame();
    frame->frame->pts = atEnd ? frameIndex : frameIndex++;
    timeInput->blockSignals(true);
    timeInput->setValue((float)frameIndex / video->frameRate);
    timeInput->blockSignals(false);
    frame->video = video;
    int maxSplits = threads.size();
    int ySplits = 1;
    if (maxSplits % 6 == 0)
        ySplits = 6;
    if (maxSplits % 4 == 0)
        ySplits = 4;
    int splitXSize = video->width / (maxSplits / ySplits);
    int splitYSize = video->height / ySplits;
    taskMutex.lock();
    for (int split = 0; split < maxSplits; split++) {
        frame->threadsLeft++;
        int splitX = split % (maxSplits / ySplits);
        int splitY = split / (maxSplits / ySplits);
        FramePreviewTask task;
        task.frame = frame;
        task.renderX = splitX * splitXSize;
        task.renderY = splitY * splitYSize;
        task.renderWidth = splitXSize;
        task.renderHeight = splitYSize;
        assert((task.renderX + task.renderWidth) <= video->width);
        assert((task.renderY + task.renderHeight) <= video->height);
        tasks.enqueue(std::move(task));
    }
    isGenerating = true;
    taskMutex.unlock();
    updateStatus();
}

void MainWindow::taskDone(FramePreviewTask task) {
    if (isClosing)
        return;
    PreviewFrame *frame = task.frame;
    frame->threadsLeft--;
    if (frame->threadsLeft > 0)
        return;
    AVFrame *frameData = frame->frame;
    QImage img = QImage(frameData->data[0], frameData->width, frameData->height,
                        frameData->linesize[0], QImage::Format_ARGB32)
                     .copy();
    previewWidget->updateImage(img);

    av_frame_free(&frame->frame);
    delete task.frame;
    isGenerating = false;

    updateStatus();
    if (tooSlow) {
        frameIndex--;
        generate();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (forceClosing)
        return;
    if (isClosing) {
        event->ignore();
        return;
    }
    isClosing = true;
    closeTimer.start();
    tasks.clear();
    if (threads.isEmpty()) {
        return;
    }
    event->ignore();
    closingDialog = new QProgressDialog("Waiting for threads to finish...", "",
                                        1, threads.size() + 1, this);
    closingDialog->setCancelButton(nullptr);
    closingDialog->setMinimumDuration(50);
    closingDialog->setValue(1);
}

void MainWindow::updateStatus() {
    if (isClosing)
        return;
    taskMutex.lock();
    QString text =
        "Threads: " + QString::number(threads.size()) +
        " | <code>frameIndex: " + QString::number(frameIndex) +
        "</code> | <code>seconds: " +
        QString::number((float)frameIndex / video->frameRate, 'f', 2) +
        "</code> | Queued: " + QString::number(tasks.size());
    taskMutex.unlock();
    if (tooSlow) {
        text +=
            " | <span style='color:orange;font-weight:bold;'>TOO SLOW!</span>";
    }
    statusText->setText(text);
}

int main(int argc, char **argv) {
    KIconTheme::initTheme();
    QApplication application(argc, argv);
    qInfo() << "pid:" << application.applicationPid();
    KStyleManager::initStyle();
    MainWindow widget;
    widget.show();
    return application.exec();
}
