#include "render_window.hpp"
#include "gui.hpp"
#include "render.hpp"
#include "scene.hpp"
#include <KMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
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
    context->width = width;
    context->height = height;
    stream->time_base = av_d2q(1. / frameRate, AV_TIME_BASE);
    stream->avg_frame_rate = av_d2q(frameRate, AV_TIME_BASE);
    context->framerate = av_d2q(frameRate, AV_TIME_BASE);
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

    lastFrameIndex = durationFrames - 1;

    QList<GuiRenderDrawThread *> threads;

    for (int i = 0; i < std::max(1, QThread::idealThreadCount() - 1); i++) {
        GuiRenderDrawThread *thread = new GuiRenderDrawThread();
        thread->guiRenderThread = this;
        thread->window = window;
        connect(thread, &GuiRenderDrawThread::errored, this,
                &GuiRenderThread::doErrored);
        thread->start();
        threads.append(thread);
    }

    emit progressed(0, lastFrameIndex);

    for (int64_t i = 0; i <= lastFrameIndex; i++) {
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

GuiRenderThread::~GuiRenderThread() {
    for (auto task : tasks) {
        delete task;
    }
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

    AVFrame *frame = av_frame_alloc();
    frame->width = width;
    frame->height = height;
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
    errorMsg = error;
    emit errored(error);
}

void GuiRenderDrawThread::run() {
    RenderThread renderThread;
    renderThread.init();

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

        FrameTask *frameTask = guiRenderThread->tasks[curFrame];
        frameTask->render(renderThread);

        swsCtx = sws_getCachedContext(
            swsCtx, frameTask->width, frameTask->height, AV_PIX_FMT_BGRA,
            frameTask->width, frameTask->height, (AVPixelFormat)frame->format,
            0, nullptr, nullptr, nullptr);

        int strides[] = {frameTask->width * 4};
        sws_scale(swsCtx, (const uint8_t *const *)&frameTask->values, strides,
                  0, frameTask->height, frame->data, frame->linesize);

        delete[] frameTask->values;

        guiRenderThread->frameMutex.lock();
        guiRenderThread->frames[curFrame] = frame;
        guiRenderThread->frameMutex.unlock();
    }

    if (swsCtx) {
        sws_freeContext(swsCtx);
    }
    renderThread.close();
}

RenderWindow::RenderWindow(NewMainWindow *mainWindow)
    : QWidget(mainWindow), mainWindow(mainWindow) {
    resize(500, 300);
    setWindowTitle("Render");
    setWindowFlag(Qt::WindowType::Window);

    encoders = {
        // prores_ks not used because it gives out of order frames and causes
        // artifacts
        {"prores", ".mov (Apple ProRes) (Recommended)", ".mov"},
        {"libx264", ".mp4 (H264), no transparency", ".mp4"},
        {"h264_nvenc", ".mp4 (H264), no transparency, NVIDIA", ".mp4"},
        {"libvpx-vp9", ".webm (VP9)", ".webm"},
    };

    auto renderLayout = new QVBoxLayout(this);
    renderLayout->setContentsMargins(8, 16, 8, 16);

    renderLayout->addWidget(new QLabel("Location:"));

    auto locationLayout = new QHBoxLayout();
    renderLayout->addLayout(locationLayout);
    renderFilePathInput = new QLineEdit(this);

    locationLayout->addWidget(renderFilePathInput);

    auto renderFilePathBrowse = new QToolButton(this);
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

    renderVideoFormatComboBox = new QComboBox(this);

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

    renderButton = new QPushButton("Render", this);
    connect(renderButton, &QPushButton::clicked, this,
            &RenderWindow::renderButtonClicked);
    renderLayout->addWidget(renderButton);

    renderedFileButton = new VideoFileButton(this);
    renderedFileButton->hide();
    renderLayout->addWidget(renderedFileButton, 0, Qt::AlignCenter);

    renderLayout->addStretch();

    renderProgressLabel = new QLabel(this);
    renderProgressLabel->hide();
    renderLayout->addWidget(renderProgressLabel);

    renderProgressBar = new QProgressBar(this);
    renderProgressBar->setRange(0, 0);
    renderProgressBar->hide();
    renderLayout->addWidget(renderProgressBar);
}

void RenderWindow::resetRenderFilePathInput() {
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

void RenderWindow::renderVideoError(QString error) {
    qCritical() << error;
    KMessageBox::error(
        this, "An error occured while rendering the video.\n\n" + error,
        "Render error");
    show();
    activateWindow();
}

void RenderWindow::renderButtonClicked() {
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
    render(fileInfo, encoder);
}

void RenderWindow::render(QFileInfo fileInfo, QString encoder) {
    renderedFileButton->hide();
    renderButton->setDisabled(true);
    renderProgressLabel->setText("Preparing...");
    renderProgressBar->setRange(0, 0);
    renderProgressLabel->show();
    renderProgressBar->show();

    thread = new GuiRenderThread(this);
    Scene *scene = mainWindow->scene;

    for (int frame = 0; frame < scene->durationFrames; frame++) {
        FrameTask *task = new FrameTask();
        task->width = scene->width;
        task->height = scene->height;
        task->frame = frame;
        task->seconds = frame / scene->frameRate;
        task->id = frame;

        for (auto element : scene->elements) {
            ElementRender *render = (ElementRender *)element->toRender({frame});
            task->renderElements.insert(task->renderElements.begin(), render);
        }

        thread->tasks.push_back(task);
    }

    thread->width = scene->width;
    thread->height = scene->height;
    thread->frameRate = scene->frameRate;
    thread->durationFrames = scene->durationFrames;
    thread->window = mainWindow;
    thread->fileInfo = fileInfo;
    thread->encoder = encoder;

    connect(thread, &GuiRenderThread::errored, this,
            &RenderWindow::renderVideoError);

    connect(thread, &GuiRenderThread::finished, thread, [this]() {
        thread->deleteLater();
        thread = nullptr;
        renderButton->setDisabled(false);
        renderProgressLabel->hide();
        renderProgressBar->hide();
        mainWindow->renderAction->setText("Render…");
    });

    connect(thread, &GuiRenderThread::progressed, thread,
            [this](int64_t frame, int64_t lastFrame) {
                renderProgressLabel->setText("Rendering (frame " +
                                             QString::number(frame) + "/" +
                                             QString::number(lastFrame) + ")");
                mainWindow->renderAction->setText(
                    QString::number((int)(((float)frame / lastFrame) * 100)) +
                    "%");
                renderProgressBar->setRange(0, lastFrame);
                renderProgressBar->setValue(frame);
            });

    connect(thread, &GuiRenderThread::finishedSuccessfully, thread,
            [this, fileInfo]() {
                renderedFileButton->show();
                renderedFileButton->setFile(fileInfo.filePath());
            });

    thread->start();
}
