#include "aegp_compute_cache_suite.hpp"
#include "ae/ae_base.hpp"

int32_t registerCacheType(char *name, void *callbacks) {
    qInfo() << "registerCacheType" << name;
    return 0;
}

AEGPComputeCacheSuite::AEGPComputeCacheSuite() {
    registerCacheType = (void *)::registerCacheType;
}
