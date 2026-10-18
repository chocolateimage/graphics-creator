#pragma once
#include "ae_base.hpp"
#include "ae_param.hpp"
#include "layer.hpp"
#include <vector>

class Plugin;
struct InData;
struct OutData;

typedef int (*effectEntryFunc_t)(uint32_t cmd, InData *inData, OutData *outData,
                                 AEParameter **params, AELayer *output,
                                 void *extra);

struct AEInternalStrings {
    std::vector<std::string> strings;
};

class AEEffect {
  public:
    enum Command {
        GlobalSetup = 1,
        ParameterSetup = 4,
        SequenceSetup = 5,
        SequenceCleanup = 8,
        DoDialog = 9,
        FrameSetup = 10,
        Render = 11,
        FrameCleanup = 12,
        Event = 15,
        SmartPrerender = 23,
        SmartRender = 24,
    };

    void registerEffect();

    Plugin *plugin;
    QString name;
    QString displayName;
    QString category;
    effectEntryFunc_t effectEntryFunc;
    QList<AEEffectParameter *> params;
    bool isSmart{false};
    bool hasDialog{false};
    QString dialogButtonText;

    AEMemoryHandle *globalData;

    bool registeredCustomUi{false};

    int runCommand(AEEffect::Command command, InData *inData, OutData *outData,
                   AEParameter **params, AELayer *output, void *extra);
};
