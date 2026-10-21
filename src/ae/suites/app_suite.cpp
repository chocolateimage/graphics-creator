#include "app_suite.hpp"
#include "gui/color_picker.hpp"
#include <QApplication>
#include <QPalette>

struct AppColor {
    enum {
        None = -1,

        Frame,
        Fill,
        Text,
        LightTinge,
        DarkTinge,
        Hilite,
        Shadow,

        ButtonFrame,
        ButtonFill,
        ButtonText,
        ButtonLightTinge,
        ButtonDarkTinge,
        ButtonHilite,
        ButtonShadow,

        ButtonPressedFrame,
        ButtonPressedFill,
        ButtonPressedText,
        ButtonPressedLightTinge,
        ButtonPressedDarkTinge,
        ButtonPressedHilite,
        ButtonPressedShadow,

        FrameDisabled,
        FillDisabled,
        TextDisabled,
        LightTingeDisabled,
        DarkTingeDisabled,
        HiliteDisabled,
        ShadowDisabled,

        ButtonFrameDisabled,
        ButtonFillDisabled,
        ButtonTextDisabled,
        ButtonLightTingeDisabled,
        ButtonDarkTingeDisabled,
        ButtonHiliteDisabled,
        ButtonShadowDisabled,

        ButtonPressedFrameDisabled,
        ButtonPressedFillDisabled,
        ButtonPressedTextDisabled,
        ButtonPressedLightTingeDisabled,
        ButtonPressedDarkTingeDisabled,
        ButtonPressedHiliteDisabled,
        ButtonPressedShadowDisabled,

        Black,
        White,
        Gray,
        Red,
        Yellow,
        Green,
        Cyan,

        TlwNeedleCurrentTime,
        TlwNeedlePreviewTime,
        TlwCacheMarkMem,
        TlwCacheMarkDisk,
        TlwCacheMarkMix,
        FillLight,
        HotText,
        HotTextDisabled,

        Label0,
        Label1,
        Label2,
        Label3,
        Label4,
        Label5,
        Label6,
        Label7,
        Label8,
        Label9,
        Label10,
        Label11,
        Label12,
        Label13,
        Label14,
        Label15,
        Label16,

        TlwCacheMarkMemDubious,
        TlwCacheMarkDiskDubious,
        TlwCacheMarkMixDubious,
        HotTextPressed,
        HotTextWarning,
        PureBlack,
        PureWhite,

        PanelBackground = 1000,
        ListBoxFill,
        DarkCaptionFill,
        DarkCaptionText,
        TextOnLighterBg,
    };

    uint16_t r, g, b;
};

int32_t appGetBgColor(AppColor *outColor) {
    QPalette palette = qApp->palette();
    QColor color = palette.window().color();
    outColor->r = color.redF() * 65535.;
    outColor->g = color.greenF() * 65535.;
    outColor->b = color.blueF() * 65535.;
    return 0;
}

int32_t appGetColor(int16_t type, AppColor *outColor) {
    QPalette palette = qApp->palette();
    QColor color;
    switch (type) {
    case AppColor::Black:
        color = Qt::black;
        break;
    case AppColor::PanelBackground:
        color = palette.color(QPalette::ColorRole::Window);
        break;
    case AppColor::ListBoxFill:
        color = palette.color(QPalette::ColorRole::Base);
        break;
    case AppColor::DarkCaptionFill:
        color = palette.color(QPalette::ColorRole::Window);
        break;
    case AppColor::Text:
    case AppColor::DarkCaptionText:
    case AppColor::TextOnLighterBg:
        color = palette.color(QPalette::ColorRole::Text);
        break;
    default: {
        qCritical() << "appGetColor() COLOR NOT IMPLEMENTED" << type;
        abort();
    } break;
    }
    outColor->r = color.redF() * 65535.;
    outColor->g = color.greenF() * 65535.;
    outColor->b = color.blueF() * 65535.;
    return 0;
}

int32_t appGetLanguage(char *) {
    qInfo() << "appGetLanguage";
    return 0;
}

int32_t getPersonalInfo(void *) {
    qInfo() << "getPersonalInfo";
    return 0;
}

int32_t getFontStylesheet(int16_t, void *, int16_t, int16_t, int16_t) {
    qInfo() << "getFontStylesheet";
    return 0;
}

int32_t setCursor(int32_t cursor) {
    qInfo() << "setCursor" << cursor;
    return 0;
}

int32_t isRenderEngine(bool *value) {
    qInfo() << "isRenderEngine";
    *value = false;
    return 0;
}

int32_t showColorPicker(char *title, PixelFloat *current, bool,
                        PixelFloat *newColor) {
    QColor color =
        QColor::fromRgbF(current->r, current->g, current->b, current->a);
    QString qTitle = title;
    ColorPickerDialog *dialog = new ColorPickerDialog(color);
    if (!qTitle.isEmpty()) {
        dialog->setWindowTitle(qTitle);
    }
    if (dialog->exec() != QDialog::Accepted) {
        return 517;
    }
    newColor->r = dialog->currentColor.redF();
    newColor->g = dialog->currentColor.greenF();
    newColor->b = dialog->currentColor.blueF();
    newColor->a = dialog->currentColor.alphaF();
    return 0;
}

int32_t getMouse() {
    qInfo() << "getMouse";
    return 0;
}
int32_t invaldiateRect() {
    qInfo() << "invaldiateRect";
    return 0;
}
int32_t convertLocalToGlobal() {
    qInfo() << "convertLocalToGlobal";
    return 0;
}
int32_t getColorAtGlobalPoint() {
    qInfo() << "getColorAtGlobalPoint";
    return 0;
}

int32_t setProjectDirty() {
    qInfo() << "setProjectDirty";
    return 0;
}
int32_t saveProject() {
    qInfo() << "saveProject";
    return 0;
}
int32_t saveBackgroundState() {
    qInfo() << "saveBackgroundState";
    return 0;
}
int32_t forceForeground() {
    qInfo() << "forceForeground";
    return 0;
}
int32_t restoreBackgroundState() {
    qInfo() << "restoreBackgroundState";
    return 0;
}
int32_t refreshAllWindows() {
    qInfo() << "refreshAllWindows";
    return 0;
}
int32_t infoDrawText2Lines() {
    qInfo() << "infoDrawText2Lines";
    return 0;
}
int32_t infoDrawColor() {
    qInfo() << "infoDrawColor";
    return 0;
}
int32_t infoDrawText3Lines() {
    qInfo() << "infoDrawText3Lines";
    return 0;
}
int32_t infoDrawText3LinesPlus() {
    qInfo() << "infoDrawText3LinesPlus";
    return 0;
}
int32_t appendInfoText() {
    qInfo() << "appendInfoText";
    return 0;
}

PFAppSuite6::PFAppSuite6() {
    appGetBgColor = (void *)::appGetBgColor;
    appGetColor = (void *)::appGetColor;
    getPersonalInfo = (void *)::getPersonalInfo;
    getFontStylesheet = (void *)::getFontStylesheet;
    setCursor = (void *)::setCursor;
    isRenderEngine = (void *)::isRenderEngine;
    showColorPicker = (void *)::showColorPicker;
    getMouse = (void *)::getMouse;
    invaldiateRect = (void *)::invaldiateRect;
    convertLocalToGlobal = (void *)::convertLocalToGlobal;
    getColorAtGlobalPoint = (void *)::getColorAtGlobalPoint;
}

PFAppSuite7::PFAppSuite7() {
    appGetBgColor = (void *)::appGetBgColor;
    appGetColor = (void *)::appGetColor;
    appGetLanguage = (void *)::appGetLanguage;
    getPersonalInfo = (void *)::getPersonalInfo;
    getFontStylesheet = (void *)::getFontStylesheet;
    setCursor = (void *)::setCursor;
    isRenderEngine = (void *)::isRenderEngine;
    showColorPicker = (void *)::showColorPicker;
}

PFAdvAppSuite2::PFAdvAppSuite2() {
    setProjectDirty = (void *)::setProjectDirty;
    saveProject = (void *)::saveProject;
    saveBackgroundState = (void *)::saveBackgroundState;
    forceForeground = (void *)::forceForeground;
    restoreBackgroundState = (void *)::restoreBackgroundState;
    refreshAllWindows = (void *)::refreshAllWindows;
    infoDrawText2Lines = (void *)::infoDrawText2Lines;
    infoDrawColor = (void *)::infoDrawColor;
    infoDrawText3Lines = (void *)::infoDrawText3Lines;
    infoDrawText3LinesPlus = (void *)::infoDrawText3LinesPlus;
    appendInfoText = (void *)::appendInfoText;
}
