#pragma once
#include "image_viewer.hpp"
#include "lua.hpp"
#include "scene.hpp"
#include "variant.hpp"
#include "video.hpp"
#include "video_file_button.hpp"
#include <DockManager.h>
#include <KMessageWidget>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <QChronoTimer>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMutex>
#include <QPointer>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QQueue>
#include <QStackedWidget>
#include <QStatusBar>
#include <QThread>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidget>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

class MainWindow;

struct EncoderInfo {
    QString encoderName;
    QString displayName;
    QString fileExtension;
};

class PreviewFrame {
  public:
    std::shared_ptr<Video> video;
    AVFrame *frame;
    std::atomic<int> threadsLeft{0};
};

class FramePreviewTask {
  public:
    PreviewFrame *frame;
    int renderX;
    int renderY;
    int renderWidth;
    int renderHeight;
};

class FramePreviewThread : public QThread {
    Q_OBJECT
  public:
    explicit FramePreviewThread(QObject *parent = nullptr) : QThread(parent) {}
    MainWindow *window{nullptr};
    std::atomic<bool> luaDirty{false};
    std::atomic<bool> optionsDirty{false};

  protected:
    void run() override;
  signals:
    void taskDone(FramePreviewTask task);
    void errored(QString error);
};

class GuiRenderThread : public QThread {
    Q_OBJECT
  public:
    explicit GuiRenderThread(QObject *parent = nullptr) : QThread(parent) {}
    MainWindow *window{nullptr};
    std::shared_ptr<Video> video;
    QString encoder;
    QFileInfo fileInfo;
    QMutex frameMutex;
    std::unordered_map<int64_t, AVFrame *> frames;
    std::vector<AVFrame *> unusedFrames;
    int64_t currentFrameIndex{0};
    int64_t lastFrameIndex{0};
    std::atomic<bool> isCancelling{false};
    std::atomic<bool> hasErrored{false};

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
    MainWindow *window{nullptr};
    std::shared_ptr<Video> video;

  protected:
    void run() override;

  signals:
    void errored(QString error);
};

struct NewTemplate {
    QString name;
    QString previewImagePath;
    QString scriptPath;
};

struct NewTemplateCategory {
    QString name;
    QList<NewTemplate> templates;
};

class VideoSettingsDialog : public QDialog {
    Q_OBJECT
  public:
    explicit VideoSettingsDialog(std::shared_ptr<Video> oldVideo,
                                 QWidget *parent = nullptr);
    std::shared_ptr<Video> oldVideo;

    std::shared_ptr<Video> video();

  private:
    QSpinBox *width;
    QSpinBox *height;
    QSpinBox *frameRate;
};

class MainWindow : public QMainWindow {
  public:
    QString dataPath = "";
    int frameIndex = 0;
    ImageViewer *previewWidget;
    QLabel *statusText;
    QChronoTimer *timer;
    std::shared_ptr<Video> video;
    QToolButton *videoControlButton;
    QToolButton *loopButton;
    std::atomic<bool> isClosing{false};
    bool forceClosing{false};
    QElapsedTimer closeTimer;
    bool isGenerating{false};
    QDoubleSpinBox *timeInput;
    QDoubleSpinBox *durationInput;

    KTextEditor::Document *textDocument;
    KTextEditor::View *textView;

    QMutex latestLuaMutex;
    std::string latestLua;
    QList<QPointer<FramePreviewThread>> threads;

    bool tooSlow{false};
    QMutex taskMutex;
    QQueue<FramePreviewTask> tasks;
    PreviewFrame *nextFrame{nullptr};

    QProgressDialog *closingDialog{nullptr};

    KMessageWidget *errorMessage;
    QString lastSetError;

    QWidget *optionsWidget;
    QFormLayout *optionsLayout;

    QMutex scriptOptionsMutex;
    std::map<std::string, Variant> scriptOptions;
    QList<std::string> addedScriptOptions;

    QAction *newAction;
    QAction *editAction;
    QAction *renderAction;
    QStackedWidget *stackedWidget;

    QComboBox *renderVideoFormatComboBox;
    QLineEdit *renderFilePathInput;
    QPushButton *renderButton;
    VideoFileButton *renderedFileButton;
    QLabel *renderProgressLabel;
    QProgressBar *renderProgressBar;

    QList<EncoderInfo> encoders;
    QList<NewTemplateCategory> categories = {};

    MainWindow();
    void loadLate();

    void loadTemplates();
    void useTemplate(const NewTemplate &newTemplate);

    void updateButtons();
    void updateTimeInput(double value);
    void updateDurationInput(double value);
    void toggleTimer();

    void updateTabs();

    void renderButtonClicked();
    void renderVideoError(QString error);
    void resetRenderFilePathInput();

    AVFrame *allocateFrame();
    void createThread();
    void generate();
    void taskDone(FramePreviewTask task);
    void updateStatus();
    void recreateOptions();
    void scriptUpdated();
    void optionsUpdated();
    void updateOption(const std::string &optionId, Variant variant);
    void updateError(const QString &error);
    bool addOptionFromLua(lua_State *L);
    void pixelPicked(QString id, QPoint position);
    void openVideoSettings();
    void videoSettingsUpdated();

    ~MainWindow() { qDebug() << "close took" << closeTimer.elapsed() << "ms"; }

  protected:
    void closeEvent(QCloseEvent *event) override;
};

class NewMainWindow : public QMainWindow {
  public:
    NewMainWindow();
    ~NewMainWindow();

    QAction *controlSelect;
    QAction *controlRectangle;
    QAction *controlEllipse;
    QAction *controlPolygon;
    QAction *controlLua;

    ImageViewer *scenePreviewWidget;

    void controlsUpdated();
    void sceneRectPicked(QString id, QRect rect);
    void renderTest();
    void rerender();
    void elementUpdated(Element *element);
    void frameChanged(int frame);

    Scene *scene;

    ads::CDockManager *dockManager;
    QUndoStack *undoStack;
};
