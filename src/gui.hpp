#include "image_viewer.hpp"
#include "lua.hpp"
#include "variant.hpp"
#include "video.hpp"
#include <KMessageWidget>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <QChronoTimer>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMutex>
#include <QPointer>
#include <QProgressDialog>
#include <QPushButton>
#include <QQueue>
#include <QStatusBar>
#include <QThread>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

class TestWindow;

class PreviewFrame {
  public:
    Video *video;
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
    TestWindow *window{nullptr};
    std::atomic<bool> luaDirty{false};
    std::atomic<bool> optionsDirty{false};

  protected:
    void run() override;
  signals:
    void taskDone(FramePreviewTask task);
    void errored(QString error);
};

// TODO: not TestWindow
class TestWindow : public QMainWindow {
  public:
    int frameIndex = 0;
    ImageViewer *previewWidget;
    QLabel *statusText;
    QChronoTimer *timer;
    Video *video;
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

    TestWindow();
    void loadLate();

    void updateButtons();
    void updateTimeInput(double value);
    void updateDurationInput(double value);
    void toggleTimer();

    AVFrame *allocateFrame();
    void createThread();
    void generate();
    void taskDone(FramePreviewTask task);
    void updateStatus();
    void scriptUpdated();
    void optionsUpdated();
    void updateOption(const std::string &optionId, Variant variant);
    void updateError(const QString &error);
    bool addOptionFromLua(lua_State *L);
    void pixelPicked(QString id, QPoint position);

    ~TestWindow() {
        delete video;
        qDebug() << "close took" << closeTimer.elapsed() << "ms";
    }

  protected:
    void closeEvent(QCloseEvent *event) override;
};