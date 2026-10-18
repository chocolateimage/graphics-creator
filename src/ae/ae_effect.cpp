#include "ae_effect.hpp"
#include "ae_data.hpp"
#include "animatable/effect/ae_plugin_effect.hpp"
#include "animatable/effect/effect_list.hpp"
#include <QMessageBox>

void AEEffect::registerEffect() {
    EffectInfo effectInfo;
    effectInfo.category = "[AE] " + category;
    effectInfo.name = "AE_" + name;
    effectInfo.displayName = displayName;
    effectInfo.create = [this]() {
        AEPluginEffect *pluginEffect = new AEPluginEffect(this);
        return pluginEffect;
    };

    InData *inData = new InData();
    OutData *outData = new OutData();
    inData->effect = this;
    qInfo() << "Global setup";
    runCommand(Command::GlobalSetup, inData, outData, nullptr, nullptr,
               nullptr);
    if (outData->flags & OutData::Obsolete) {
        effectInfo.displayName += " [DEPRECATED]";
    }
    if (outData->flags & OutData::HasDialogs) {
        hasDialog = true;
    }
    if (outData->flags2 & OutData::Smart) {
        isSmart = true;
    }
    qInfo() << "Param setup";
    runCommand(Command::ParameterSetup, inData, outData, nullptr, nullptr,
               nullptr);
    delete inData;
    delete outData;

    effectList.push_back(effectInfo);
}

int AEEffect::runCommand(AEEffect::Command command, InData *inData,
                         OutData *outData, AEParameter **params,
                         AELayer *output, void *extra) {
    inData->globalData = globalData;
    outData->globalData = globalData;
    inData->downsampleX = {1, 1};
    inData->downsampleY = {1, 1};
    inData->pixelAspectRatio = {1, 1};
    inData->applicationId = 1180193859;
    outData->returnMessage[0] = 0;
    int ret = effectEntryFunc(command, inData, outData, params, output, extra);
    if (outData->flags & OutData::ErrorMessage ||
        outData->returnMessage[0] != 0) {
        QString msg = outData->returnMessage;
        qCritical() << "Plugin message from" << displayName << ":" << msg;
        if (outData->flags & OutData::ErrorMessage) {
            QMessageBox::critical(nullptr, displayName, msg);
        } else {
            QMessageBox::information(nullptr, displayName, msg);
        }
    }
    if (outData->globalData != nullptr) {
        globalData = outData->globalData;
    }
    if (ret != 0) {
        qWarning() << "Error from running plugin effect" << ret;
    }
    return ret;
}
