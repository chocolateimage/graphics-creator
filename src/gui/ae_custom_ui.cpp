#include "ae_custom_ui.hpp"
#include "ae/ae_data.hpp"
#include "ae/ae_effect.hpp"
#include "ae/ae_param.hpp"
#include "animatable/effect/ae_plugin_effect.hpp"
#include "gui.hpp"
#include "variant.hpp"
#include <QMouseEvent>
#include <QPainter>

AECustomUIWindow::AECustomUIWindow(AEPluginEffect *effect) : effect(effect) {
    qInfo() << "custom ui widget created";
    inData = new InData();
    outData = new OutData();
    context = new AEContext();
    context->windowType = AEContext::WindowType::Effect;
    context->drawRef = this;

    AEEventExtra extra{};
    extra.eventType = AEEventExtra::EventType::NewContext;
    sendEventToPlugin(&extra);
}

void AECustomUIWindow::sendEventToPlugin(AEEventExtra *extra) {
    extra->context = &context;
    extra->effectWin.effectArea = AEEffectWindowInfo::EffectArea::Control;
    extra->effectWin.currentFrame = {0, 0, width(), height()};
    extra->outFlags = 0;
    inData->sequenceData = effect->sequenceData;
    outData->sequenceData = effect->sequenceData;

    qInfo() << "SENDING EVENT TO PLUGIN" << extra->eventType;
    AEPluginEffectRender *renderEffect =
        (AEPluginEffectRender *)effect->toRender(
            {0}); // TODO: IMPORTANT: frameindex!!!!!
    renderEffect->prepare();
    effect->effect->runCommand(AEEffect::Command::Event, inData, outData,
                               renderEffect->params.data(), nullptr, extra);
    if (outData->sequenceData) {
        effect->sequenceData = outData->sequenceData;
    }
    if ((extra->outFlags & AEEventExtra::OutFlags::UpdateNow) ||
        (extra->outFlags & AEEventExtra::OutFlags::AlwaysUpdate)) {
        NewMainWindow::instance()->invalidateAndRerender();
    }
    delete renderEffect;
}
void AECustomUIWindow::paintEvent(QPaintEvent *event) {
    painter = new QPainter(this);
    AEEventExtra extra{};
    extra.eventType = AEEventExtra::EventType::Draw;
    extra.info.draw.depth = 8;
    extra.info.draw.updateRect = {0, 0, width(), height()};
    sendEventToPlugin(&extra);
    delete painter;
}

void AECustomUIWindow::mousePressEvent(QMouseEvent *event) {
    AEEventExtra extra{};
    extra.eventType = AEEventExtra::EventType::DoClick;
    extra.info.click.numClicks = 1;
    extra.info.click.sendDrag = false;
    extra.info.click.when = 0;
    extra.info.click.lastTime = 0;
    QPoint pos = event->position().toPoint();
    extra.info.click.screenPoint = {pos.x(), pos.y()};
    sendEventToPlugin(&extra);
}

AECustomUIWindow::~AECustomUIWindow() {
    delete inData;
    delete outData;
    delete context;
}
