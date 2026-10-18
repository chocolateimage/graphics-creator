#pragma once
#include "ae/ae_base.hpp"

struct PFChannelSuite1 {
    PFChannelSuite1();

    void *getLayerChannelCount;
    void *getLayerChannelIndexedRefAndDesc;
    void *getLayerChannelTypedRefAndDesc;
    void *checkoutLayerChannel;
    void *checkinLayerChannel;
};
