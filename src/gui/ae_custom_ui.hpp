#pragma once
#include <QWidget>

class AEPluginEffect;
struct InData;
struct OutData;
struct AEEventExtra;
struct AEContext;

class AECustomUIWindow : public QWidget {
    Q_OBJECT
  public:
    explicit AECustomUIWindow(AEPluginEffect *effect);
    ~AECustomUIWindow();

    void sendEventToPlugin(AEEventExtra *extra);

    AEPluginEffect *effect;
    InData *inData;
    OutData *outData;
    AEContext *context;
    QPainter *painter;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};
