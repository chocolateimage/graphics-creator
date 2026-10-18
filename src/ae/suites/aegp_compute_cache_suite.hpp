#pragma once
#include "ae/ae_base.hpp"

struct AEGPComputeCacheSuite {
    AEGPComputeCacheSuite();

    void *registerCacheType;
    void *unregisterCacheType;
    void *computeIfNeededAndCheckout;
    void *checkoutCached;
    void *getReceiptComputeValue;
    void *checkinComputeReceipt;
};
