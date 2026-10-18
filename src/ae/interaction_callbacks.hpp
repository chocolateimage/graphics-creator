#pragma once
#include "ae_base.hpp"
#include "ae_effect.hpp"
#include "ae_param.hpp"

class AEPluginEffectRender;

struct AECustomUIInfo {
    int32_t reserved;
    int32_t customEventFlags;
    int32_t uiCompWidth;
    int32_t uiCompHeight;
    int32_t uiCompAlignment;
    int32_t uiLayerWidth;
    int32_t uiLayerHeight;
    int32_t uiLayerAlignment;
    int32_t uiPreviewWidth;
    int32_t uiPreviewHeight;
    int32_t uiPreviewAlignment;
};

struct InteractionCallbacks {
    InteractionCallbacks();

    int (*inquireParam)(AEPluginEffectRender *, int64_t, int64_t, int64_t,
                        uint64_t, AEParameter *);
    int (*inquireParamClose)(void *, AEParameter *);
    int (*addParam)(AEEffect *, int, AEParameter *);
    int (*abort)(void *);
    int (*progress)(void *, int32_t, int32_t);
    int (*registerUi)(AEEffect *, AECustomUIInfo *);
    void *func7{nullptr};
    void *func8{nullptr};
    void *func9{nullptr};
    void *func10{nullptr};
    void *func11{nullptr};
    void *func12{nullptr};
    uint32_t (*initData)(char ***, uint32_t);
    char *(*getString)(void *, uint8_t, uint8_t, uint8_t);
    void (*cleanupData)(void *);
    void *func16{nullptr};
    void *func17{nullptr};
    void *func18{nullptr};
    void *func19{nullptr};
    void *func20{nullptr};
    void *func21{nullptr};
    void *func22{nullptr};
};
