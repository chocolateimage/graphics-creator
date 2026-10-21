#pragma once
#include <QtSystemDetection>
#ifdef Q_OS_WIN
#include <QWidget>
#include <windows.h>

class AEGPPanelRegistration;
struct Vector2DInt;

struct PopupMenuItem {
    enum ItemType {
        Normal,
        Checked,
        Radio,
        Separator,
    };

    int32_t indent;
    ItemType type;
    bool enabled;
    int32_t commandId;
    char *name;
};

class PopupMenuItem2 {
  public:
    PopupMenuItem *info;
    HMENU menu;
    int32_t indent;
    PopupMenuItem2 *parent;
    QList<PopupMenuItem2 *> children;
};

struct AEPanelFunctions {
    int32_t (*getSnapSizes)(void *instanceRefcon, Vector2DInt *snapSizes,
                            int32_t *sizes);
    int32_t (*populatePopup)(void *instanceRefcon, PopupMenuItem *items,
                             int32_t *itemCount);
    int32_t (*doPopupCommand)(void *instanceRefcon, int32_t commandId);
};

class AEPanelWindow : public QWidget {
    Q_OBJECT
  public:
    explicit AEPanelWindow(AEGPPanelRegistration *registration);
    AEGPPanelRegistration *registration;
    void *instanceRefcon;
    AEPanelFunctions functions;

    int lastItemId;
    QMap<int, int> menuItemCommandMapping;
    bool created = false;

    void createPopup();
    void createPopupItems(PopupMenuItem2 *item);

  protected:
    bool nativeEvent(const QByteArray &eventType, void *message,
                     qintptr *result) override;
    void showEvent(QShowEvent *event) override;
};
#endif
