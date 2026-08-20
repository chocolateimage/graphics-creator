#pragma once
#include "video_file_button.hpp"
#include <QComboBox>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMutex>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class NewMainWindow;
class FrameTask;

struct EncoderInfo {
    QString encoderName;
    QString displayName;
    QString fileExtension;
};

class GuiRenderThread : public QThread {
    Q_OBJECT
  public:
    explicit GuiRenderThread(QObject *parent = nullptr) : QThread(parent) {}
    ~GuiRenderThread();
    NewMainWindow *window{nullptr};
    QString encoder;
    QFileInfo fileInfo;
    QMutex frameMutex;
    std::unordered_map<int64_t, AVFrame *> frames;
    std::vector<AVFrame *> unusedFrames;
    int64_t currentFrameIndex{0};
    int64_t lastFrameIndex{0};
    std::atomic<bool> isCancelling{false};
    std::atomic<bool> hasErrored{false};
    QString errorMsg;

    std::vector<FrameTask *> tasks{};

    int width;
    int height;
    float frameRate;
    int durationFrames;

    AVFrame *getFrame();

  protected:
    void run() override;

  private:
    AVFormatContext *formatContext{nullptr};
    AVDictionary *opt{nullptr};
    const AVCodec *codec{nullptr};
    AVCodecContext *context{nullptr};
    AVPacket *tempPacket{nullptr};
    AVStream *stream{nullptr};

    bool writeFrame(AVFrame *frame);

  public slots:
    void doErrored(QString error);

  signals:
    void errored(QString error);
    void progressed(int64_t frame, int64_t lastFrame);
    void finishedSuccessfully();
};

class GuiRenderDrawThread : public QThread {
    Q_OBJECT
  public:
    explicit GuiRenderDrawThread(QObject *parent = nullptr) : QThread(parent) {}
    GuiRenderThread *guiRenderThread{nullptr};
    NewMainWindow *window{nullptr};
    SwsContext *swsCtx{nullptr};

  protected:
    void run() override;

  signals:
    void errored(QString error);
};

class RenderWindow : public QWidget {
    Q_OBJECT
  public:
    explicit RenderWindow(NewMainWindow *mainWindow);

    NewMainWindow *mainWindow;
    QList<EncoderInfo> encoders;

    QComboBox *renderVideoFormatComboBox;
    QLineEdit *renderFilePathInput;
    QPushButton *renderButton;
    QLabel *renderProgressLabel;
    QProgressBar *renderProgressBar;
    VideoFileButton *renderedFileButton;

    GuiRenderThread *thread{nullptr};

    void resetRenderFilePathInput();
    void renderButtonClicked();
    void render(QFileInfo fileInfo, QString encoder);

  public slots:
    void renderVideoError(QString error);
};
