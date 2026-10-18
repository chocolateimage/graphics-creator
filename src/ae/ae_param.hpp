#pragma once
#include "ae_base.hpp"
#include "layer.hpp"
#include "variant.hpp"

class AEEffect;

struct AEFixedSlider {
    Fixed value;
    char str[32];
    char description[32];
    Fixed min;
    Fixed max;
    Fixed min2;
    Fixed max2;
    Fixed defaultValue;
    int16_t decimalPlaces;
};
struct AEFloatSlider {
    double value;
    double phase;
    char description[32];
    float min;
    float max;
    float min2;
    float max2;
    float defaultValue;
};
struct AECheckbox {
    uint32_t checked;
    bool defaultValue;
    char reserved[3];
    char *name;
};
struct AEPopup {
    int32_t value;
    int16_t choiceCount;
    int16_t defaultValue;
    char *names;
};
struct AEColor {
    ARGB value;
    ARGB defaultValue;
};
struct AEAngle {
    Fixed value;
    Fixed defaultValue;
};
struct AEParamPoint {
    Fixed x;
    Fixed y;
    char reserved[4];
    Fixed defaultX;
    Fixed defaultY;
};
struct AESlider {
    int32_t value;
    char str[32];
    char description[32];
    int32_t min;
    int32_t max;
    int32_t min2;
    int32_t max2;
    int32_t defaultValue;
};
struct AEButton {
    int32_t unused;
    char *text;
};
struct AECustomData {
    // TODO: use the arbritrary commands to save/load/copy/interp data
    int16_t id;
    int16_t unused;
    AEMemoryHandle *defaultValue;
    AEMemoryHandle *value;
    void *refcon;
};

struct AEParameter {
    enum ParamType {
        Layer,
        Slider,
        FixedSlider,
        Angle,
        Checkbox,
        Color,
        Point,
        Popup,
        unused1,
        NoData,
        FloatSlider,
        Data,
        Path,
        GroupStart,
        GroupEnd,
        Button,
        unused3,
        unused4,
    };
    enum ParamFlags {
        Supervise = 1 << 6, // NOT IMPLEMENTED
    };
    enum ParamUIFlags {
        None = 0,
        Topic = 1 << 0,
        Control = 1 << 1,
        StdControl = 1 << 2,
        NoEcwUi = 1 << 3,
        EcwSeparator = 1 << 4,
        Disabled = 1 << 5,
        DontEraseTopic = 1 << 6,
        DontEraseControl = 1 << 7,
        RadioButton = 1 << 8,
        Invisible = 1 << 9,
    };

    int32_t id;
    int32_t uiFlags;
    int16_t uiWidth;
    int16_t uiHeight;
    ParamType type;
    char name[32];
    uint32_t flags;
    uint32_t unuseda2;
    union {
        AELayer layer;
        AEFixedSlider fixedSlider;
        AEFloatSlider floatSlider;
        AECheckbox checkbox;
        AEPopup popup;
        AEColor color;
        AEAngle angle;
        AEParamPoint point;
        AESlider slider;
        AEButton button;
        AECustomData customData;

        char data[120];
    };
};

class AEEffectParameter {
  public:
    AEEffect *effect;
    AEParameter::ParamType type;
    QString name;
    VariantType min;
    VariantType max;
    VariantType defaultValue;
    QList<QString> choices;
    QString buttonText;
    int32_t uiFlags;
    int16_t uiWidth;
    int16_t uiHeight;
};

struct AEContext {
    enum WindowType {
        None = -1,
        Comp,
        Layer,
        Effect,
        Preview,
    };

    uint32_t magic;
    WindowType windowType;
    void *reserved;
    void *something[4];
    void *drawRef;
    void *reserved3;
    void *reserved4;
};
struct AEEventFunctions {
    void *data;
    void *layerToComp;
    void *compToLayer;
    void *getComp2LayerXform;
    void *getLayer2CompXform;
    void *sourceToFrame;
    void *frameToSource;
    void *infoDrawColor;
    void *infoDrawText;
};
struct AEEffectWindowInfo {
    enum EffectArea {
        None,
        ParamTitle,
        Control,
    };
    int32_t paramIndex;
    EffectArea effectArea;
    AERect currentFrame;
    AERect paramTitleFrame;
    int32_t horizontalOffset;
};
struct AEItemWindowInfo {
    char unused[16];
};

class AEModifiers {
  public:
    enum Enum {
        None,
    };
};

struct AEEventDrawInfo {
    AERect updateRect;
    int32_t depth;
};
struct AEEventClickInfo {
    uint32_t when;
    Vector2DInt screenPoint;
    int32_t numClicks;
    AEModifiers::Enum modifiers;
    void *something[4];
    bool sendDrag;
    bool lastTime;
};
struct AEEventKeyDownInfo {
    uint32_t when;
    Vector2DInt screenPoint;
    uint32_t keyCode;
    AEModifiers::Enum modifiers;
};
struct AEAdjustCursorInfo {
    Vector2DInt screenPoint;
    AEModifiers::Enum modifiers;
    int32_t newCursor;
};

struct AEEventExtra {
    enum EventType {
        EventTypeNone = -1,
        NewContext,
        Activate,
        DoClick,
        Drag,
        Draw,
        Deactivate,
        CloseContext,
        Idle,
        unused,
        AdjustCursor,
        Keydown,
        MouseExited,
    };
    enum OutFlags {
        OutFlagNone = 0,
        HandledEvent = 1 << 0,
        AlwaysUpdate = 1 << 1,
        NeverUpdate = 1 << 2,
        UpdateNow = 1 << 3,
    };

    AEContext **context;
    EventType eventType;
    union {
        AEEventDrawInfo draw;
        AEEventClickInfo click;
        AEEventKeyDownInfo keyDown;
        AEAdjustCursorInfo adjustCursor;
    } info;
    union {
        AEEffectWindowInfo effectWin;
        AEItemWindowInfo itemWin;
    };
    AEEventFunctions functions;
    int32_t inFlags;
    int32_t outFlags;
};
