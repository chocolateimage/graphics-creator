#include "effect_list.hpp"
#include "blur_effect.hpp"
#include "crt_effect.hpp"
#include "grid_effect.hpp"
#include "invert_effect.hpp"
#include "scale_effect.hpp"
#include "wave_effect.hpp"

const std::vector<EffectInfo> effectList = {
    DEFINE_EFFECT("Blur", "Box Blur", "boxBlur", BlurEffect),

    DEFINE_EFFECT("Tools", "Invert", "invert", InvertEffect),
    DEFINE_EFFECT("Tools", "Scale", "scale", ScaleEffect),

    DEFINE_EFFECT("Style", "Wave", "wave", WaveEffect),
    DEFINE_EFFECT("Style", "MattiasCRT", "mattiasCrt", CrtEffect),

    DEFINE_EFFECT("Generate", "Grid", "grid", GridEffect),
};
