#include "world_transform_suite.hpp"
#include "ae/utility_callbacks.hpp"

PFWorldTransformSuite1::PFWorldTransformSuite1() {
    blend = ::blend;
    copy = ::copy;
    copy2 = ::copy;
    transferRect = (void *)::transferRect;
}
