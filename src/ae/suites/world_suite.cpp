#include "world_suite.hpp"
#include "ae/utility_callbacks.hpp"

int32_t createWorld_wrapper(void *effect, int32_t w, int32_t h, bool clear,
                            int32_t pixelFormat, AELayer *layer) {
    qInfo() << "createWorld_wrapper";
    return createWorld(effect, w, h, pixelFormat, layer);
}
int getPixelFormat(void *world, PixelFormat *pixelFormat) {
    qInfo() << "getPixelFormat";
    qInfo() << "WORLD" << world;
    *pixelFormat = PixelFormat::ARGB32;
    return 0;
}

PFWorldSuite2::PFWorldSuite2() {
    createWorld = ::createWorld_wrapper;
    deleteWorld = ::deleteWorld;
    getPixelFormat = ::getPixelFormat;
}
