#include "suite_loader.hpp"
#include "suites/aegp_compute_cache_suite.hpp"
#include "suites/aegp_memory_suite.hpp"
#include "suites/aegp_pf_interface_suite.hpp"
#include "suites/aegp_register_suite.hpp"
#include "suites/aegp_utility_suite.hpp"
#include "suites/aegp_world_suite.hpp"
#include "suites/ansi_suite.hpp"
#include "suites/app_suite.hpp"
#include "suites/channel_suite.hpp"
#include "suites/color_param_suite.hpp"
#include "suites/drawbot_suites.hpp"
#include "suites/effect_ui_suite.hpp"
#include "suites/fill_matte_suite.hpp"
#include "suites/handle_suite.hpp"
#include "suites/iterate_suite.hpp"
#include "suites/param_utils_suite.hpp"
#include "suites/world_suite.hpp"
#include "suites/world_transform_suite.hpp"

PFAnsiSuite2 *ansiSuite = new PFAnsiSuite2();
PFWorldSuite2 *worldSuite = new PFWorldSuite2();
PFWorldTransformSuite1 *worldTransformSuite = new PFWorldTransformSuite1();
PFFillMatteSuite2 *fillMatteSuite = new PFFillMatteSuite2();
PFIterate8Suite1 *iterateSuite1 = new PFIterate8Suite1();
PFIterate8Suite2 *iterateSuite2 = new PFIterate8Suite2();
PFHandleSuite2 *handleSuite2 = new PFHandleSuite2();
AEGPUtilitySuite3 *aegpUtilitySuite3 = new AEGPUtilitySuite3();
AEGPUtilitySuite6 *aegpUtilitySuite6 = new AEGPUtilitySuite6();
AEGPMemorySuite *aegpMemorySuite = new AEGPMemorySuite();
PFAppSuite6 *appSuite6 = new PFAppSuite6();
PFAppSuite7 *appSuite7 = new PFAppSuite7();
PFAdvAppSuite2 *advAppSuite2 = new PFAdvAppSuite2();
AEGPComputeCacheSuite *aegpComputeCacheSuite = new AEGPComputeCacheSuite();
PFEffectUISuite *effectUiSuite = new PFEffectUISuite();
AEGPRegisterSuite *aegpRegisterSuite = new AEGPRegisterSuite();
PFParamUtilsSuite3 *paramUtilsSuite3 = new PFParamUtilsSuite3();
PFColorParamSuite1 *colorParamSuite1 = new PFColorParamSuite1();
PFChannelSuite1 *channelSuite1 = new PFChannelSuite1();
PFIterate16Suite2 *iterate16Suite2 = new PFIterate16Suite2();
PFIterateFloatSuite2 *iterateFloatSuite2 = new PFIterateFloatSuite2();
AEGPPFInterfaceSuite1 *aegpInterfaceSuite1 = new AEGPPFInterfaceSuite1();
DrawbotDrawSuite1 *drawbotDrawSuite1 = new DrawbotDrawSuite1();
DrawbotSupplierSuite1 *drawbotSupplierSuite1 = new DrawbotSupplierSuite1();
DrawbotSurfaceSuite2 *drawbotSurfaceSuite2 = new DrawbotSurfaceSuite2();
DrawbotPathSuite1 *drawbotPathSuite1 = new DrawbotPathSuite1();
PFEffectCustomUISuite1 *effectCustomUISuite1 = new PFEffectCustomUISuite1();
AEGPWorldSuite3 *aegpWorldSuite3 = new AEGPWorldSuite3();

uint32_t loadSuite(const char *name, int32_t version, const void **suite) {
    // qInfo() << "loadSuite";
    // qInfo() << "name:" << name;
    // qInfo() << "version:" << version;
    if (strcmp(name, "PF ANSI Suite") == 0 && version == 2) {
        *suite = ansiSuite;
    } else if (strcmp(name, "PF ANSI Suite") == 0 && version == 1) {
        *suite = ansiSuite;
    } else if (strcmp(name, "PF World Suite") == 0 && version == 2) {
        *suite = worldSuite;
    } else if (strcmp(name, "PF World Transform Suite") == 0 && version == 1) {
        *suite = worldTransformSuite;
    } else if (strcmp(name, "PF Fill Matte Suite") == 0 && version == 2) {
        *suite = fillMatteSuite;
    } else if (strcmp(name, "PF Iterate8 Suite") == 0 && version == 1) {
        *suite = iterateSuite1;
    } else if (strcmp(name, "PF Iterate8 Suite") == 0 && version == 2) {
        *suite = iterateSuite2;
    } else if (strcmp(name, "PF Handle Suite") == 0 && version == 2) {
        *suite = handleSuite2;
    } else if (strcmp(name, "AEGP Utility Suite") == 0 && version == 7) {
        *suite = aegpUtilitySuite3;
    } else if (strcmp(name, "AEGP Utility Suite") == 0 && version == 13) {
        *suite = aegpUtilitySuite6;
    } else if (strcmp(name, "AEGP Memory Suite") == 0 && version == 1) {
        *suite = aegpMemorySuite;
    } else if (strcmp(name, "PF AE App Suite") == 0 && version == 6) {
        *suite = appSuite6;
    } else if (strcmp(name, "PF AE App Suite") == 0 && version == 7) {
        *suite = appSuite7;
    } else if (strcmp(name, "AEGP Compute Cache") == 0 && version == 1) {
        *suite = aegpComputeCacheSuite;
    } else if (strcmp(name, "PF Effect UI Suite") == 0 && version == 1) {
        *suite = effectUiSuite;
    } else if (strcmp(name, "AEGP Register Suite") == 0 && version == 6) {
        *suite = aegpRegisterSuite;
    } else if (strcmp(name, "PF Param Utils Suite") == 0 && version == 3) {
        *suite = paramUtilsSuite3;
    } else if (strcmp(name, "PF ColorParamSuite") == 0 && version == 1) {
        *suite = colorParamSuite1;
    } else if (strcmp(name, "PF AE Channel Suite") == 0 && version == 1) {
        *suite = channelSuite1;
    } else if (strcmp(name, "PF iterateFloat Suite") == 0 && version == 2) {
        *suite = iterateFloatSuite2;
    } else if (strcmp(name, "PF iterate16 Suite") == 0 && version == 2) {
        *suite = iterate16Suite2;
    } else if (strcmp(name, "AEGP PF Interface Suite") == 0 && version == 1) {
        *suite = aegpInterfaceSuite1;
    } else if (strcmp(name, "DRAWBOT Draw Suite") == 0 && version == 1) {
        *suite = drawbotDrawSuite1;
    } else if (strcmp(name, "DRAWBOT Supplier Suite") == 0 && version == 1) {
        *suite = drawbotSupplierSuite1;
    } else if (strcmp(name, "DRAWBOT Surface Suite") == 0 &&
               (version == 1 || version == 2)) {
        *suite = drawbotSurfaceSuite2;
    } else if (strcmp(name, "DRAWBOT Path Suite") == 0 && version == 1) {
        *suite = drawbotPathSuite1;
    } else if (strcmp(name, "PF Effect Custom UI Suite") == 0 && version == 1) {
        *suite = effectCustomUISuite1;
    } else if (strcmp(name, "AEGP World Suite") == 0 && version == 3) {
        *suite = aegpWorldSuite3;
    } else if (strcmp(name, "AEGP World Suite") == 0 && version == 3) {
        *suite = aegpWorldSuite3;
    } else if (strcmp(name, "PF AE Adv App Suite") == 0 && version == 2) {
        *suite = advAppSuite2;
    } else {
        qInfo() << "NO SUITE FOUND!" << name << version;
        // abort();
        return 13;
    }
    return 0;
}

uint32_t unloadSuite(const char *name, int32_t version) {
    // qInfo() << "unloadSuite";
    // qInfo() << "name:" << name;
    // qInfo() << "version:" << version;
    // qInfo() << "NOT IMPLEMENTED YET!";
    return 0;
}
bool isEqual(const char *token1, const char *token2) {
    qInfo() << "IsEqual";
    qInfo() << "token1:" << token1;
    qInfo() << "token2:" << token2;
    return strcmp(token1, token2) == 0;
}
uint32_t allocateBlock(size_t size, void **block) {
    qInfo() << "AllocateBlock";
    *block = malloc(size);
    return size;
}
uint32_t freeBlock(void *block) {
    qInfo() << "FreeBlock";
    free(block);
    return 0;
}
uint32_t reallocateBlock(void *block, size_t newSize, void **newblock) {
    qInfo() << "ReallocateBlock";
    abort();
    return 0;
}

SuiteLoader::SuiteLoader() {
    loadSuite = ::loadSuite;
    unloadSuite = ::unloadSuite;
    isEqual = ::isEqual;
    allocateBlock = ::allocateBlock;
    freeBlock = ::freeBlock;
    reallocateBlock = ::reallocateBlock;
}
