#include "gui.hpp"
#include "brush_input.hpp"
#include "draggable_spinbox.hpp"
#include "fontcombobox.hpp"
#include "line.hpp"
#include "lua.hpp"
#include "lua_code.hpp"
#include "lua_state.hpp"
#include "math.hpp"
#include "render.hpp"
#include "variant.hpp"
#include <DockAreaWidget.h>
#include <KActionCollection>
#include <KActionMenu>
#include <KColorButton>
#include <KIconTheme>
#include <KMessageBox>
#include <KMessageWidget>
#include <KStyleManager>
#include <KTextEditor/View>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFrame>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QScrollArea>
#include <QSlider>
#include <QSpacerItem>
#include <QSplitter>
#include <QStandardPaths>
#include <QToolBar>
#include <QToolButton>
#include <fontconfig/fontconfig.h>

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
        std::shared_ptr<Video> video = task.frame->video;

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

void GuiRenderThread::run() {
    qInfo() << "starting rendering";
    QString filePath = fileInfo.filePath();
    QDir().mkpath(fileInfo.absolutePath());

    avformat_alloc_output_context2(&formatContext, nullptr, nullptr,
                                   qPrintable(filePath));
    if (!formatContext) {
        doErrored("output context could not be created");
        return;
    }

    // add_stream()
    codec = avcodec_find_encoder_by_name(qPrintable(encoder));
    if (!codec) {
        doErrored("codec not found");
        return;
    }
    context = avcodec_alloc_context3(codec);
    if (!context) {
        doErrored("context could not be created");
        return;
    }
    tempPacket = av_packet_alloc();
    if (!context) {
        doErrored("packet could not be created");
        return;
    }
    stream = avformat_new_stream(formatContext, nullptr);
    if (!context) {
        doErrored("stream could not be created");
        return;
    }

    av_opt_set(context->priv_data, "crf", "30", 0);
    av_opt_set(context->priv_data, "qp", "30", 0);

    context->codec_id = codec->id;
    context->width = video->width;
    context->height = video->height;
    stream->time_base = {1, (int)video->frameRate};
    stream->avg_frame_rate = {(int)video->frameRate, 1};
    context->framerate = {(int)video->frameRate, 1};
    context->time_base = stream->time_base;
    context->thread_count = 0;
    context->gop_size = 20;
    context->pix_fmt = AV_PIX_FMT_YUV420P;
    if (context->codec_id == AV_CODEC_ID_QTRLE) {
        context->pix_fmt = AV_PIX_FMT_ARGB;
    } else if (context->codec_id == AV_CODEC_ID_PRORES) {
        context->pix_fmt = AV_PIX_FMT_YUVA444P10LE;
    } else if (context->codec_id == AV_CODEC_ID_UTVIDEO) {
        context->pix_fmt = AV_PIX_FMT_GBRAP;
    } else if (context->codec_id == AV_CODEC_ID_VP9) {
        context->pix_fmt = AV_PIX_FMT_YUVA420P;
    }

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // open_video()
    int ret = avcodec_open2(context, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        doErrored(QStringLiteral("could not open video codec: ") +
                  av_err2str(ret));
        return;
    }

    ret = avcodec_parameters_from_context(stream->codecpar, context);
    if (ret < 0) {
        doErrored(QStringLiteral("could not copy stream params: ") +
                  av_err2str(ret));
        return;
    }

    // ---
    if (!(formatContext->flags & AVFMT_NOFILE)) {
        ret = avio_open(&formatContext->pb, qPrintable(filePath),
                        AVIO_FLAG_WRITE);
        if (ret < 0) {
            doErrored(QStringLiteral("could not open file: ") +
                      av_err2str(ret));
            return;
        }
    }

    ret = avformat_write_header(formatContext, &opt);
    if (ret < 0) {
        doErrored(QStringLiteral("could not write header: ") + av_err2str(ret));
        return;
    }

    int64_t frameCount = (video->duration * video->frameRate) - 1;
    lastFrameIndex = frameCount;

    QList<GuiRenderDrawThread *> threads;

    for (int i = 0; i < std::max(1, QThread::idealThreadCount() - 1); i++) {
        GuiRenderDrawThread *thread = new GuiRenderDrawThread();
        thread->guiRenderThread = this;
        thread->window = window;
        thread->video = video;
        connect(thread, &GuiRenderDrawThread::errored, this,
                &GuiRenderThread::doErrored);
        thread->start();
        threads.append(thread);
    }

    emit progressed(0, lastFrameIndex);

    for (int64_t i = 0; i <= frameCount; i++) {
        if (isCancelling)
            break;

        frameMutex.lock();
        auto frame = frames.find(i);
        if (frame == frames.end()) {
            frameMutex.unlock();
            QThread::usleep(500);
            i--;
            continue;
        }

        emit progressed(i, lastFrameIndex);

        AVFrame *avFrame = frame->second;
        frames.erase(frame);
        frameMutex.unlock();

        qInfo() << i;

        bool continueRunning = false;

        continueRunning = writeFrame(avFrame);

        frameMutex.lock();
        unusedFrames.push_back(avFrame);
        frameMutex.unlock();

        if (!continueRunning)
            break;
    }
    writeFrame(nullptr);

    for (auto thread : threads) {
        thread->wait();
        delete thread;
    }
    threads.clear();

    for (auto frame : unusedFrames) {
        av_frame_free(&frame);
    }
    unusedFrames.clear();

    av_write_trailer(formatContext);

    avcodec_free_context(&context);
    av_packet_free(&tempPacket);

    if (!(formatContext->flags & AVFMT_NOFILE)) {
        avio_closep(&formatContext->pb);
    }
    avformat_free_context(formatContext);

    if (!hasErrored) {
        emit finishedSuccessfully();
    }

    qInfo() << "finished rendering";
}

bool GuiRenderThread::writeFrame(AVFrame *frame) {
    int ret = avcodec_send_frame(context, frame);
    if (ret < 0) {
        doErrored(QStringLiteral("error sending frame to encoder: ") +
                  av_err2str(ret));
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(context, tempPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            doErrored(QStringLiteral("error encoding frame: ") +
                      av_err2str(ret));
            return false;
        }

        av_packet_rescale_ts(tempPacket, context->time_base, stream->time_base);
        tempPacket->stream_index = stream->index;
        ret = av_interleaved_write_frame(formatContext, tempPacket);
        if (ret < 0) {
            doErrored(QStringLiteral("error writing output packet: ") +
                      av_err2str(ret));
            return false;
        }
    }

    return true;
}

AVFrame *GuiRenderThread::getFrame() {
    QMutexLocker locker(&frameMutex);
    if (!unusedFrames.empty()) {
        AVFrame *frame = unusedFrames.back();
        unusedFrames.pop_back();
        av_frame_make_writable(frame);
        return frame;
    }

    AVFrame *frame = video->allocateFrame();
    frame->format = context->pix_fmt;
    av_frame_get_buffer(frame, 0);
    av_frame_make_writable(frame);
    return frame;
}

void GuiRenderThread::doErrored(QString error) {
    if (isCancelling)
        return;
    isCancelling = true;
    hasErrored = true;
    emit errored(error);
}

void GuiRenderDrawThread::run() {
    RenderThread renderThread;
    renderThread.init();

    window->latestLuaMutex.lock();
    bool error = !renderThread.loadLua(window->latestLua);
    window->latestLuaMutex.unlock();
    if (error) {
        emit errored(QString::fromStdString(renderThread.lastError));
        return;
    }

    window->scriptOptionsMutex.lock();
    renderThread.updateOptions(window->scriptOptions);
    window->scriptOptionsMutex.unlock();

    while (!guiRenderThread->isCancelling) {
        guiRenderThread->frameMutex.lock();

        if (guiRenderThread->frames.size() > 300) {
            guiRenderThread->frameMutex.unlock();
            QThread::msleep(1);
            continue;
        }

        if (guiRenderThread->currentFrameIndex >
            guiRenderThread->lastFrameIndex) {
            guiRenderThread->frameMutex.unlock();
            break;
        }

        int64_t curFrame = guiRenderThread->currentFrameIndex++;
        guiRenderThread->frameMutex.unlock();

        AVFrame *frame = guiRenderThread->getFrame();
        frame->pts = curFrame;

        if (!renderThread.drawImage(video, frame, 0, 0, video->width,
                                    video->height)) {
            emit errored(QString::fromStdString(renderThread.lastError));
            break;
        }

        guiRenderThread->frameMutex.lock();
        guiRenderThread->frames[curFrame] = frame;
        guiRenderThread->frameMutex.unlock();
    }

    renderThread.close();
}

MainWindow::MainWindow() : QMainWindow() {
    QElapsedTimer measure;
    measure.start();
    this->setCursor(Qt::WaitCursor);
    this->resize(1200, 700);
    QStringList dataPaths = {
        QApplication::applicationDirPath() + "/data",
        QApplication::applicationDirPath() + "/../share/graphics-creator/data",
    };

    for (auto path : dataPaths) {
        if (QDir(path).exists() &&
            QFileInfo(path + "/templates/LICENSE").exists()) {
            dataPath = QDir(path).absolutePath();
            break;
        }
    }

    if (dataPath.isEmpty()) {
        qCritical() << "No data path found. Tried:" << dataPaths;
        KMessageBox::error(nullptr, "No data path found");
        exit(1);
    }

    QTimer::singleShot(4, this, &MainWindow::loadLate);
    qDebug() << "init took" << measure.elapsed() << "ms";
}

VideoSettingsDialog::VideoSettingsDialog(std::shared_ptr<Video> oldVideo,
                                         QWidget *parent)
    : QDialog(parent), oldVideo(oldVideo) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Video Settings");
    auto parentLay = new QVBoxLayout(this);
    auto lay = new QFormLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    parentLay->addLayout(lay);

    width = new DraggableSpinBox(this);
    width->setRange(1, 999999);
    width->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    height = new DraggableSpinBox(this);
    height->setRange(1, 999999);
    height->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    frameRate = new DraggableSpinBox(this);
    frameRate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frameRate->setRange(1, 999999);

    width->setValue(oldVideo->width);
    height->setValue(oldVideo->height);
    frameRate->setValue(oldVideo->frameRate);

    lay->addRow("Width", width);
    lay->addRow("Height", height);
    lay->addRow("Frame Rate", frameRate);
    setMinimumWidth(400);

    parentLay->addStretch();

    auto buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    parentLay->addWidget(buttonBox);
}

std::shared_ptr<Video> VideoSettingsDialog::video() {
    auto video = std::make_shared<Video>(*oldVideo);
    video->frameRate = frameRate->value();
    video->width = width->value();
    video->height = height->value();
    return video;
}

void MainWindow::loadLate() {
    QElapsedTimer measure;
    measure.start();

    const FcChar8 *fontsToMatch[] = {
        (const FcChar8 *)"Noto Sans:regular:slant=0",
        (const FcChar8 *)"Arial:regular:slant=0",
    };

    for (auto fontName : fontsToMatch) {
        FcPattern *pattern = FcNameParse(fontName);
        FcResult result;
        FcPattern *font = FcFontMatch(nullptr, pattern, &result);
        if (result != FcResultMatch || !font) {
            FcPatternDestroy(pattern);
            continue;
        }

        FcChar8 *rawFileName;
        FcChar8 *rawFamily;
        FcChar8 *rawStyle;
        int fontIndex;
        FcPatternGetString(font, FC_FILE, 0, &rawFileName);
        FcPatternGetInteger(font, FC_INDEX, 0, &fontIndex);
        FcPatternGetString(font, FC_FAMILY, 0, &rawFamily);
        FcPatternGetString(font, FC_STYLE, 0, &rawStyle);

        Variant::defaultFont = {std::string((char *)rawFileName), fontIndex,
                                std::string((char *)rawFamily) + " " +
                                    std::string((char *)rawStyle)};

        FcPatternDestroy(font);

        FcPatternDestroy(pattern);

        break;
    }

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
        if (timer->isActive()) {
            toggleTimer();
        }
        textDocument->setText(LUA_BASIC_CODE);
    });
    connect(editAction, &QAction::triggered, this, [this]() {
        stackedWidget->setCurrentIndex(1);
        updateTabs();
    });
    connect(renderAction, &QAction::triggered, this, [this]() {
        if (timer->isActive()) {
            toggleTimer();
        }
        stackedWidget->setCurrentIndex(2);
        updateTabs();
        resetRenderFilePathInput();
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

    loadTemplates();

    auto editCentralWidget = new QWidget(stackedWidget);
    stackedWidget->addWidget(editCentralWidget);
    auto topLayout = new QVBoxLayout(editCentralWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);
    auto splitter = new QSplitter(this);
    topLayout->addWidget(splitter, 1);

    auto videoContentWidget = new QWidget();
    auto videoContentLayout = new QVBoxLayout(videoContentWidget);
    videoContentLayout->setAlignment(Qt::AlignmentFlag::AlignBottom);
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
    videoContentLayout->addWidget(previewWidget, 1);

    auto bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    videoContentLayout->addLayout(bottomLayout);

    // TODO: a slider or something to control the time
    timeInput = new DraggableDoubleSpinBox(this);
    durationInput = new DraggableDoubleSpinBox(this);
    timeInput->setFixedWidth(100);
    timeInput->setSuffix(" s");
    timeInput->setSingleStep(0.5);
    durationInput->setFixedWidth(100);
    durationInput->setMaximum(INFINITY);
    durationInput->setSuffix(" s");
    connect(timeInput, &DraggableDoubleSpinBox::valueChanged, this,
            &MainWindow::updateTimeInput);
    connect(durationInput, &DraggableDoubleSpinBox::valueChanged, this,
            &MainWindow::updateDurationInput);
    bottomLayout->addWidget(videoControlButton);
    bottomLayout->addWidget(new QLabel("Time:", this));
    bottomLayout->addWidget(timeInput);
    bottomLayout->addWidget(new QLabel("Duration:", this));
    bottomLayout->addWidget(durationInput);
    bottomLayout->addWidget(loopButton);
    bottomLayout->addStretch();

    QToolButton *videoSettingsButton = new QToolButton();
    videoSettingsButton->setText("Settings");
    videoSettingsButton->setToolTip("Change video settings");
    videoSettingsButton->setIcon(QIcon::fromTheme("settings-configure"));
    videoSettingsButton->setToolButtonStyle(
        Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    connect(videoSettingsButton, &QToolButton::clicked, this,
            &MainWindow::openVideoSettings);
    bottomLayout->addWidget(videoSettingsButton);

    auto rightSideWidget = new QWidget();
    auto rightSideLayout = new QVBoxLayout(rightSideWidget);
    rightSideLayout->setContentsMargins(0, 0, 0, 0);
    auto rightSideSplitter = new QSplitter(rightSideWidget);
    rightSideSplitter->setOrientation(Qt::Orientation::Vertical);
    rightSideLayout->addWidget(rightSideSplitter);
    splitter->addWidget(rightSideWidget);

    auto scrollArea = new QScrollArea(rightSideSplitter);
    optionsWidget = new QWidget();
    optionsLayout = new QFormLayout(optionsWidget);
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    scrollArea->setWidget(optionsWidget);
    scrollArea->setWidgetResizable(true);
    rightSideSplitter->addWidget(scrollArea);

    auto editor = KTextEditor::Editor::instance();
    textDocument = editor->createDocument(this);
    textDocument->setMode("Lua");
    textDocument->setConfigValue("indent-pasted-text", "false");
    textDocument->setText(LUA_BASIC_CODE);
    textView = textDocument->createView(rightSideSplitter);
#ifdef Q_OS_WIN
    textView->setConfigValue("font", "Consolas");
#endif
    textView->setConfigValue("theme", "VSCodium Dark");
    textView->setStatusBarEnabled(false);
    textView->setContextMenu(
        textView->defaultContextMenu()); // what else should I use??
    for (auto actionCollection :
         textDocument->actionCollection()->allCollections()) {
        for (auto action : actionCollection->actions()) {
            if (action->objectName().startsWith("file_save")) {
                action->setDisabled(true);
            }
        }
    }
    connect(textDocument, &KTextEditor::Document::textChanged, this,
            &MainWindow::scriptUpdated);
    rightSideSplitter->addWidget(textView);

    rightSideSplitter->setSizes({50, 50});

    errorMessage = new KMessageWidget(this);
    errorMessage->setCloseButtonVisible(false);
    errorMessage->setVisible(false);
    errorMessage->setMessageType(KMessageWidget::MessageType::Error);
    rightSideLayout->addWidget(errorMessage);

    splitter->setSizes({500, 400});

    // TODO: use QDateTime::currentMSecsSinceEpoch() or QElapsedTimer for better
    // precision instead of relying on microsecond timers
    timer = new QChronoTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QChronoTimer::timeout, this, &MainWindow::generate);

    updateButtons();

    auto renderWidget = new QScrollArea(stackedWidget);
    renderWidget->setAlignment(Qt::AlignmentFlag::AlignTop |
                               Qt::AlignmentFlag::AlignHCenter);
    auto renderContent = new QWidget(renderWidget);
    renderContent->setMaximumWidth(700);
    renderWidget->setWidgetResizable(true);
    renderWidget->setWidget(renderContent);
    auto renderLayout = new QVBoxLayout(renderContent);
    renderLayout->setContentsMargins(8, 16, 8, 16);

    renderLayout->addWidget(new QLabel("Location:"));

    auto locationLayout = new QHBoxLayout();
    renderLayout->addLayout(locationLayout);
    renderFilePathInput = new QLineEdit(renderContent);

    encoders = {
        // prores_ks not used because it gives out of order frames and causes
        // artifacts
        {"prores", ".mov (Apple ProRes) (Recommended)", ".mov"},
        {"libx264", ".mp4 (H264), no transparency", ".mp4"},
        {"h264_nvenc", ".mp4 (H264), no transparency, NVIDIA", ".mp4"},
        {"libsvtav1", ".webm (AV1)", ".webm"},
    };

    locationLayout->addWidget(renderFilePathInput);

    auto renderFilePathBrowse = new QToolButton(renderContent);
    renderFilePathBrowse->setIcon(QIcon::fromTheme("document-open-data"));
    renderFilePathBrowse->setToolTip("Browse…");
    connect(renderFilePathBrowse, &QToolButton::clicked, this, [this]() {
        QString extension =
            encoders[renderVideoFormatComboBox->currentIndex()].fileExtension;
        QString filePath = QFileDialog::getSaveFileName(
            this, "Select video location", "",
            extension + " format (*" + extension + ");;All files (*.*)");
        if (filePath.isEmpty())
            return;

        renderFilePathInput->setText(filePath);
    });
    locationLayout->addWidget(renderFilePathBrowse);

    renderLayout->addWidget(new QLabel("Video format:"));

    renderVideoFormatComboBox = new QComboBox(renderContent);

    for (const auto &encoder : encoders) {
        const AVCodec *codec =
            avcodec_find_encoder_by_name(qPrintable(encoder.encoderName));
        if (!codec) {
            continue;
        }

        renderVideoFormatComboBox->addItem(
            encoder.displayName, QVariant::fromValue(encoder.encoderName));
    }

    resetRenderFilePathInput();

    connect(renderVideoFormatComboBox, &QComboBox::currentIndexChanged, this,
            [this](int index) {
                auto splitted = renderFilePathInput->text().split(".");
                if (splitted.size() > 1) {
                    splitted.takeLast();
                }
                QString beginning = splitted.join(".");
                QString final = beginning + encoders[index].fileExtension;
                renderFilePathInput->setText(final);
            });

    renderLayout->addWidget(renderVideoFormatComboBox);

    renderLayout->addSpacing(8);

    renderButton = new QPushButton("Render", renderContent);
    connect(renderButton, &QPushButton::clicked, this,
            &MainWindow::renderButtonClicked);
    renderLayout->addWidget(renderButton);

    renderLayout->addSpacing(16);

    renderedFileButton = new VideoFileButton(renderContent);
    renderedFileButton->hide();
    renderLayout->addWidget(renderedFileButton, 0, Qt::AlignCenter);

    renderLayout->addStretch();

    renderProgressLabel = new QLabel(renderContent);
    renderProgressLabel->hide();
    renderLayout->addWidget(renderProgressLabel);

    renderProgressBar = new QProgressBar(renderContent);
    renderProgressBar->setRange(0, 0);
    renderProgressBar->hide();
    renderLayout->addWidget(renderProgressBar);

    stackedWidget->addWidget(renderWidget);

    updateTabs();

    // TODO: allow user to set their own thread count
    for (int i = 0; i < std::max(1, QThread::idealThreadCount() - 2); i++) {
        createThread();
    }

    this->unsetCursor();
    qDebug() << "late init took" << measure.elapsed() << "ms";
}

void MainWindow::openVideoSettings() {
    VideoSettingsDialog *dialog = new VideoSettingsDialog(video, this);
    connect(dialog, &VideoSettingsDialog::accepted, this, [this, dialog]() {
        video = dialog->video();
        videoSettingsUpdated();
    });
    dialog->open();
}

void MainWindow::videoSettingsUpdated() {
    timer->setInterval(
        std::chrono::microseconds((int)((1000.f * 1000.f) / video->frameRate)));
}

void MainWindow::loadTemplates() {
    QScrollArea *newPageScrollArea = new QScrollArea(stackedWidget);
    newPageScrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    stackedWidget->addWidget(newPageScrollArea);

    QFile categoriesFile(dataPath + "/templates/categories.json");
    if (!categoriesFile.open(QIODevice::ReadOnly)) {
        KMessageBox::error(nullptr, "Error loading templates");
        return;
    }

    QByteArray categoriesData = categoriesFile.readAll();
    QJsonDocument categoriesDoc = QJsonDocument::fromJson(categoriesData);
    categoriesFile.close();

    for (const auto &categoryJson : categoriesDoc.array()) {
        const auto &categoryObject = categoryJson.toObject();

        NewTemplateCategory category;
        category.name = categoryObject["name"].toString();

        for (const auto &templateJson : categoryObject["templates"].toArray()) {
            const auto &categoryJson = templateJson.toObject();

            NewTemplate newTemplate;
            newTemplate.name = categoryJson["name"].toString();
            QString fileName =
                dataPath + "/templates/" + categoryJson["file"].toString();
            newTemplate.scriptPath = fileName + ".lua";
            newTemplate.previewImagePath = fileName + ".png";
            category.templates.append(newTemplate);
        }

        categories.append(category);
    }

    QWidget *newPage = new QWidget(newPageScrollArea);
    newPage->setFixedWidth(800);
    newPageScrollArea->setWidgetResizable(true);
    newPageScrollArea->setWidget(newPage);
    QVBoxLayout *newPageLayout = new QVBoxLayout(newPage);
    newPageLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    for (const auto &category : categories) {
        newPageLayout->addSpacing(16);

        QLabel *categoryLabel = new QLabel(newPage);
        categoryLabel->setText(category.name);
        categoryLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        categoryLabel->setStyleSheet("font-weight: bold");
        newPageLayout->addWidget(categoryLabel);

        QGridLayout *categoryLayout = new QGridLayout();
        categoryLayout->setAlignment(Qt::AlignLeft);

        int i = 0;
        for (const auto &newTemplate : category.templates) {
            QPushButton *btn = new QPushButton(newPage);
            btn->setFixedWidth(newPage->width() / 5 - 10);
            btn->setFixedHeight(130);
            auto btnLay = new QVBoxLayout(btn);
            btnLay->setSpacing(0);

            QPixmap previewPixmap;
            if (!previewPixmap.load(newTemplate.previewImagePath)) {
                qWarning() << "Failed to load" << newTemplate.previewImagePath;
                previewPixmap = QPixmap(150, 84);
                previewPixmap.fill(QColor(20, 20, 20));
                QPainter painter(&previewPixmap);
                painter.setPen(Qt::white);
                painter.setBrush(Qt::NoBrush);
                painter.drawText(0, 0, 150, 84, Qt::AlignCenter,
                                 "Error loading preview");
            } else {
                previewPixmap.setDevicePixelRatio(2);
            }

            auto previewImage = new QLabel(btn);
            previewImage->setPixmap(previewPixmap);
            previewImage->setAlignment(Qt::AlignCenter);
            btnLay->addWidget(previewImage);

            auto label = new QLabel(newTemplate.name, btn);
            label->setWordWrap(true);
            label->setAlignment(Qt::AlignCenter);
            btnLay->addWidget(label);

            connect(btn, &QPushButton::clicked, this,
                    [this, newTemplate]() { useTemplate(newTemplate); });

            int row = i / 5;
            int column = i % 5;
            categoryLayout->addWidget(btn, i / 5, i % 5);
            i++;
        }

        newPageLayout->addLayout(categoryLayout);
    }
}

void MainWindow::useTemplate(const NewTemplate &newTemplate) {
    QFile scriptFile(newTemplate.scriptPath);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        KMessageBox::error(nullptr, "Error loading script file" +
                                        newTemplate.scriptPath);
        return;
    }

    video = std::make_shared<Video>();
    video->frameRate = 60;
    video->width = 1920;
    video->height = 1080;
    video->duration = 5;
    videoSettingsUpdated();

    QByteArray scriptData = scriptFile.readAll();
    scriptFile.close();

    textDocument->setText(scriptData);
    textView->setCursorPosition({0, 0});
    durationInput->setValue(5);
    timeInput->setValue(0);
    generate();

    stackedWidget->setCurrentIndex(1);
    updateTabs();

    toggleTimer();
}

void MainWindow::resetRenderFilePathInput() {
    QDateTime now = QDateTime::currentDateTime();
    QString savedFolder =
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) +
        QDir::separator() + "Graphics";
    QString fileName = now.toString("yyyy-MM-dd hh-mm-ss");
    QString extension =
        encoders[renderVideoFormatComboBox->currentIndex()].fileExtension;
    renderFilePathInput->setText(savedFolder + QDir::separator() + fileName +
                                 extension);
}

void MainWindow::renderVideoError(QString error) {
    qCritical() << error;
    KMessageBox::error(
        this, "An error occured while rendering the video.\n\n" + error,
        "Render error");
}

void MainWindow::renderButtonClicked() {
    QFileInfo fileInfo = QFileInfo(renderFilePathInput->text());
    QString encoder = renderVideoFormatComboBox->currentData().toString();

    if (fileInfo.exists()) {
        if (KMessageBox::warningTwoActions(
                this,
                "The file \"" + fileInfo.filePath() +
                    "\" already exists. Do you want to overwrite it?",
                "Overwrite file?", KStandardGuiItem::overwrite(),
                KStandardGuiItem::cancel()) != KMessageBox::PrimaryAction) {
            return;
        }
    }

    renderedFileButton->hide();
    renderButton->setDisabled(true);
    renderProgressLabel->setText("Preparing...");
    renderProgressBar->setRange(0, 0);
    renderProgressLabel->show();
    renderProgressBar->show();

    GuiRenderThread *thread = new GuiRenderThread(this);
    thread->window = this;
    thread->video = video;
    thread->fileInfo = fileInfo;
    thread->encoder = encoder;

    thread->start();

    connect(thread, &GuiRenderThread::errored, this,
            &MainWindow::renderVideoError);

    connect(thread, &GuiRenderThread::finished, thread, [this, thread]() {
        thread->deleteLater();
        renderButton->setDisabled(false);
        renderProgressLabel->hide();
        renderProgressBar->hide();
    });

    connect(thread, &GuiRenderThread::progressed, thread,
            [this](int64_t frame, int64_t lastFrame) {
                renderProgressLabel->setText("Rendering (frame " +
                                             QString::number(frame) + "/" +
                                             QString::number(lastFrame) + ")");
                renderProgressBar->setRange(0, lastFrame);
                renderProgressBar->setValue(frame);
            });

    connect(thread, &GuiRenderThread::finishedSuccessfully, thread,
            [this, fileInfo]() {
                renderedFileButton->show();
                renderedFileButton->setFile(fileInfo.filePath());
            });
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

    statusBar()->setVisible(currentIndex == 1);

    newAction->blockSignals(false);
    editAction->blockSignals(false);
    renderAction->blockSignals(false);
}

void MainWindow::updateTimeInput(double value) {
    frameIndex = value * video->frameRate;
    updateStatus();
    generate();
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
        auto input = new QPlainTextEdit(optionsWidget);
        input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        input->setFixedHeight(50);
        input->setPlainText(QString::fromStdString(variant.get<std::string>()));
        connect(input, &QPlainTextEdit::textChanged, this,
                [this, optionId, input]() {
                    updateOption(optionId,
                                 Variant(input->toPlainText().toStdString()));
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
            auto input = new DraggableSpinBox(optionsWidget);
            input->setMinimum(min);
            input->setMaximum(max);
            input->setValue(variant.get<int>());
            connect(input, &DraggableSpinBox::valueChanged, this,
                    [this, optionId](int value) {
                        updateOption(optionId, Variant(value));
                    });
            widget = input;
        }
    } else if (optionType == VariantTypeEnum::Double) {
        auto input = new DraggableDoubleSpinBox(optionsWidget);
        input->setMinimum(min);
        input->setMaximum(max);
        input->setValue(variant.get<double>());
        connect(input, &DraggableDoubleSpinBox::valueChanged, this,
                [this, optionId](double value) {
                    updateOption(optionId, Variant(value));
                });
        widget = input;
    } else if (optionType == VariantTypeEnum::Vector2DInt) {
        widget = new QWidget(optionsWidget);
        auto layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        auto value = variant.get<Vector2DInt>();

        auto inputX = new DraggableSpinBox(optionsWidget);
        inputX->setMinimum(min);
        inputX->setMaximum(max);
        inputX->setValue(value.x);

        auto inputY = new DraggableSpinBox(optionsWidget);
        inputY->setMinimum(min);
        inputY->setMaximum(max);
        inputY->setValue(value.y);

        auto pickButton = new QToolButton(optionsWidget);
        pickButton->setToolTip("Pick position from preview");
        pickButton->setIcon(QIcon::fromTheme("select"));

        layout->addWidget(inputX);
        layout->addWidget(inputY);
        layout->addWidget(pickButton);

        connect(inputX, &DraggableSpinBox::valueChanged, this,
                [this, optionId, inputY](int value) {
                    updateOption(optionId, Variant((Vector2DInt){
                                               value, inputY->value()}));
                });
        connect(inputY, &DraggableSpinBox::valueChanged, this,
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
    } else if (optionType == VariantTypeEnum::Bool) {
        auto input = new QCheckBox(optionsWidget);
        input->setChecked(variant.get<bool>());
        connect(input, &QCheckBox::checkStateChanged, this,
                [this, optionId](Qt::CheckState checkState) {
                    updateOption(optionId, Variant(checkState ==
                                                   Qt::CheckState::Checked));
                });
        widget = input;
    } else if (optionType == VariantTypeEnum::Easing) {
        std::vector<std::function<double(double)>> functions = {
            linear,         easeInQuad,     easeOutQuad,    easeInOutQuad,
            easeInCubic,    easeOutCubic,   easeInOutCubic, easeInQuart,
            easeOutQuart,   easeInOutQuart, easeInQuint,    easeOutQuint,
            easeInOutQuint, easeInSine,     easeOutSine,    easeInOutSine,
            easeInExpo,     easeOutExpo,    easeInOutExpo,  easeInCirc,
            easeOutCirc,    easeInOutCirc,  easeInBack,     easeOutBack,
            easeInOutBack,  easeInElastic,  easeOutElastic, easeInOutElastic,
            easeInBounce,   easeOutBounce,  easeInOutBounce};

        std::vector<std::string> names = {"",

                                          "easeInQuad",
                                          "easeOutQuad",
                                          "easeInOutQuad",
                                          "easeInCubic",
                                          "easeOutCubic",
                                          "easeInOutCubic",
                                          "easeInQuart",
                                          "easeOutQuart",
                                          "easeInOutQuart",
                                          "easeInQuint",
                                          "easeOutQuint",
                                          "easeInOutQuint",
                                          "easeInSine",
                                          "easeOutSine",
                                          "easeInOutSine",
                                          "easeInExpo",
                                          "easeOutExpo",
                                          "easeInOutExpo",
                                          "easeInCirc",
                                          "easeOutCirc",
                                          "easeInOutCirc",
                                          "easeInBack",
                                          "easeOutBack",
                                          "easeInOutBack",
                                          "easeInElastic",
                                          "easeOutElastic",
                                          "easeInOutElastic",
                                          "easeInBounce",
                                          "easeOutBounce",
                                          "easeInOutBounce"};

        QStringList displayNames = {
            "Linear/Constant",

            "Quad In",         "Quad Out",     "Quad In Out",    "Cubic In",
            "Cubic Out",       "Cubic In Out", "Quart In",       "Quart Out",
            "Quart In Out",    "Quint In",     "Quint Out",      "Quint In Out",
            "Sine In",         "Sine Out",     "Sine In Out",    "Expo In",
            "Expo Out",        "Expo In Out",  "Circ In",        "Circ Out",
            "Circ In Out",     "Back In",      "Back Out",       "Back In Out",
            "Elastic In",      "Elastic Out",  "Elastic In Out", "Bounce In",
            "Bounce Out",      "Bounce In Out"};
        auto input = new QComboBox(optionsWidget);

        for (size_t i = 0; i < names.size(); i++) {
            QPixmap pixmap(24, 24);
            pixmap.fill(Qt::transparent);
            QPainter pixmapPainter(&pixmap);
            pixmapPainter.setPen(Qt::NoPen);
            for (int x = 0; x < pixmap.width(); x++) {
                double xValue = functions[i]((double)x / pixmap.width());
                if (xValue > 1 || xValue < 0) {
                    pixmapPainter.setBrush(QColor(255, 100, 100));
                } else {
                    pixmapPainter.setBrush(palette().text());
                }
                int xHeight = xValue * pixmap.height();
                pixmapPainter.drawRect(x, pixmap.height() - xHeight, 1,
                                       xHeight);
            }
            pixmapPainter.end();
            QIcon previewIcon(pixmap);

            input->addItem(previewIcon, displayNames[i]);
        }

        auto value = variant.get<Easing>();
        input->setCurrentIndex(
            std::find(names.begin(), names.end(), value.easingCurve) -
            names.begin());
        connect(input, &QComboBox::currentIndexChanged, this,
                [this, optionId, names](int index) {
                    updateOption(optionId, Variant(Easing{names[index]}));
                });
        widget = input;
    } else if (optionType == VariantTypeEnum::Brush) {
        auto input = new BrushInput(optionsWidget);
        Brush value = variant.get<Brush>();
        input->setValue(value);
        connect(input, &BrushInput::valueChanged, this,
                [this, optionId](Brush newValue) {
                    updateOption(optionId, Variant(newValue));
                });
        widget = input;
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
    bool incrementFrame = timer->isActive();
    if (atEnd && incrementFrame) {
        if (shouldLoop) {
            frameIndex = 0;
        } else {
            toggleTimer();
            frameIndex = video->duration * video->frameRate;
            incrementFrame = false;
        }
    }
    tooSlow = false;
    PreviewFrame *frame = new PreviewFrame();
    frame->frame = allocateFrame();
    frame->frame->pts = incrementFrame ? frameIndex++ : frameIndex;
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
    if (!video)
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

NewMainWindow::NewMainWindow() : QMainWindow() {
    this->resize(1200, 700);

    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize,
                                     true);

    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *view = menuBar->addMenu("View");
    setMenuBar(menuBar);

    QToolBar *toolBar = addToolBar("Controls");
    QActionGroup *controlsGroup = new QActionGroup(toolBar);
    QAction *controlSelect =
        toolBar->addAction(QIcon::fromTheme("select"), "Select");
    toolBar->addSeparator();
    QAction *controlRectangle =
        toolBar->addAction(QIcon::fromTheme("draw-rectangle"), "Rectangle");
    QAction *controlEllipse =
        toolBar->addAction(QIcon::fromTheme("draw-circle"), "Ellipse");
    QAction *controlPolygon =
        toolBar->addAction(QIcon::fromTheme("draw-polygon"), "Polygon");
    QAction *controlLua =
        toolBar->addAction(QIcon::fromTheme("scriptnew"), "Lua");

    controlSelect->setActionGroup(controlsGroup);
    controlRectangle->setActionGroup(controlsGroup);
    controlEllipse->setActionGroup(controlsGroup);
    controlPolygon->setActionGroup(controlsGroup);
    controlLua->setActionGroup(controlsGroup);

    controlSelect->setCheckable(true);
    controlRectangle->setCheckable(true);
    controlEllipse->setCheckable(true);
    controlPolygon->setCheckable(true);
    controlLua->setCheckable(true);

    controlSelect->setChecked(true);

    dockManager = new ads::CDockManager(this);

    ads::CDockWidget *widgetTest1 = dockManager->createDockWidget("Scene");
    // QLabel *test1 = new QLabel("This is the scene");
    ImageViewer *imageViewer = new ImageViewer();
    QPixmap pix(1280, 720);
    pix.fill(Qt::transparent);
    {
        QPainter painter(&pix);
        painter.setRenderHint(QPainter::RenderHint::Antialiasing);
        painter.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::RenderHint::TextAntialiasing);
        painter.setBrush(Qt::red);
        painter.setPen(QPen(Qt::white, 64));
        painter.drawRect(50, 50, 400, 400);
    }
    imageViewer->updateImage(pix.toImage());
    widgetTest1->setWidget(imageViewer);

    ads::CDockWidget *widgetTest2 = dockManager->createDockWidget("Timeline");
    QLabel *test2 = new QLabel("Here you can animate elements");
    widgetTest2->setWidget(test2);

    ads::CDockWidget *widgetTest3 = dockManager->createDockWidget("Properties");
    QLabel *test3 =
        new QLabel("here you can change the properties of an elemenet");
    test3->setWordWrap(true);
    widgetTest3->setWidget(test3);

    ads::CDockWidget *effectsDockWidget =
        dockManager->createDockWidget("Effects");
    QLabel *test4 = new QLabel("effects on an element");
    test4->setWordWrap(true);
    effectsDockWidget->setWidget(test4);

    auto scene = dockManager->addDockWidget(
        ads::DockWidgetArea::CenterDockWidgetArea, widgetTest1);
    auto elements = dockManager->addDockWidget(
        ads::DockWidgetArea::RightDockWidgetArea, widgetTest3);
    auto effects = dockManager->addDockWidget(
        ads::DockWidgetArea::BottomDockWidgetArea, effectsDockWidget, elements);
    auto timeline = dockManager->addDockWidget(
        ads::DockWidgetArea::BottomDockWidgetArea, widgetTest2);
    QSizePolicy policy = scene->sizePolicy();
    policy.setHorizontalStretch(1);
    policy.setVerticalStretch(1);
    scene->setSizePolicy(policy);
    policy = elements->sizePolicy();
    policy.setHorizontalStretch(0);
    policy.setVerticalStretch(1);
    elements->setSizePolicy(policy);
    policy = timeline->sizePolicy();
    policy.setHorizontalStretch(1);
    policy.setVerticalStretch(0);
    timeline->setSizePolicy(policy);

    view->addAction(widgetTest1->toggleViewAction());
    view->addAction(widgetTest2->toggleViewAction());
    view->addAction(widgetTest3->toggleViewAction());
    view->addAction(effectsDockWidget->toggleViewAction());
}

NewMainWindow::~NewMainWindow() {}

int main(int argc, char **argv) {
    KIconTheme::initTheme();
    QApplication application(argc, argv);
    qInfo() << "pid:" << application.applicationPid();
    application.setDesktopFileName("me.chocolateimage.graphics-creator");
    application.setApplicationDisplayName(QStringLiteral("Graphics Creator"));
    KStyleManager::initStyle();
    NewMainWindow widget;
    widget.show();
    return application.exec();
}
