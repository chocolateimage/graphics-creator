#pragma once
#include "ae_base.hpp"

struct AELayer {
    void *reserved[2];
    uint32_t flags;
    char unused1[4];
    ARGB *data;
    uint32_t stride;
    uint32_t width;
    uint32_t height;
    AERect rect;
    char unused3[4];
    void *unused;
    uint32_t reserved2;
    char unused4[4];
    void *reserved3;
    Scale pixelAspectRatio;
    void *reserved4;
    int32_t originX;
    int32_t originY;
    int32_t reserved5;
    int32_t default_;
} __attribute__((packed));
