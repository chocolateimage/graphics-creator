#pragma once

static inline float mix(float t, float min, float max) {
    return (t * (max - min)) + min;
}
