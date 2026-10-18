#include "aegp_world_suite.hpp"

int32_t create() {
    qInfo() << "create";
    abort();
    return 0;
}
int32_t dispose() {
    qInfo() << "dispose";
    abort();
    return 0;
}
int32_t getType() {
    qInfo() << "getType";
    abort();
    return 0;
}
int32_t getSize() {
    qInfo() << "getSize";
    abort();
    return 0;
}
int32_t getRowBytes() {
    qInfo() << "getRowBytes";
    abort();
    return 0;
}
int32_t getBaseAddress8() {
    qInfo() << "getBaseAddress8";
    abort();
    return 0;
}
int32_t getBaseAddress16() {
    qInfo() << "getBaseAddress16";
    abort();
    return 0;
}
int32_t getBaseAddress32() {
    qInfo() << "getBaseAddress32";
    abort();
    return 0;
}
int32_t fillOutWorld() {
    qInfo() << "fillOutWorld";
    abort();
    return 0;
}
int32_t fastBlur() {
    qInfo() << "fastBlur";
    abort();
    return 0;
}
int32_t createPlatformWorld() {
    qInfo() << "createPlatformWorld";
    abort();
    return 0;
}
int32_t disposePlatformWorld() {
    qInfo() << "disposePlatformWorld";
    abort();
    return 0;
}
int32_t newReferenceFromPlatformWorld() {
    qInfo() << "newReferenceFromPlatformWorld";
    abort();
    return 0;
}

AEGPWorldSuite3::AEGPWorldSuite3() {
    create = (void *)::create;
    dispose = (void *)::dispose;
    getType = (void *)::getType;
    getSize = (void *)::getSize;
    getRowBytes = (void *)::getRowBytes;
    getBaseAddress8 = (void *)::getBaseAddress8;
    getBaseAddress16 = (void *)::getBaseAddress16;
    getBaseAddress32 = (void *)::getBaseAddress32;
    fillOutWorld = (void *)::fillOutWorld;
    fastBlur = (void *)::fastBlur;
    createPlatformWorld = (void *)::createPlatformWorld;
    disposePlatformWorld = (void *)::disposePlatformWorld;
    newReferenceFromPlatformWorld = (void *)::newReferenceFromPlatformWorld;
}
