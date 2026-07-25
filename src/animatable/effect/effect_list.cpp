#include "effect_list.hpp"
#include "blur_effect.hpp"
#include "brightness_effect.hpp"
#include "crop_effect.hpp"
#include "crt_effect.hpp"
#include "drop_shadow_effect.hpp"
#include "flip_effect.hpp"
#include "grid_effect.hpp"
#include "invert_effect.hpp"
#include "long_shadow_effect.hpp"
#include "offset_effect.hpp"
#include "opacity_effect.hpp"
#include "pixelate_effect.hpp"
#include "place_original_effect.hpp"
#include "scale_effect.hpp"
#include "threshold_effect.hpp"
#include "tint_effect.hpp"
#include "twist_effect.hpp"
#include "wave_effect.hpp"

const std::vector<EffectInfo> effectList = {
    DEFINE_EFFECT("Blur", "Box Blur", "boxBlur", BlurEffect),

    DEFINE_EFFECT("Tools", "Invert", "invert", InvertEffect),
    DEFINE_EFFECT("Tools", "Offset", "offset", OffsetEffect),
    DEFINE_EFFECT("Tools", "Scale", "scale", ScaleEffect),
    DEFINE_EFFECT("Tools", "Opacity", "opacity", OpacityEffect),
    DEFINE_EFFECT("Tools", "Brightness", "brightness", BrightnessEffect),
    DEFINE_EFFECT("Tools", "Tint", "tint", TintEffect),
    DEFINE_EFFECT("Tools", "Flip", "flip", FlipEffect),
    DEFINE_EFFECT("Tools", "Place Original", "placeOriginal",
                  PlaceOriginalEffect),
    DEFINE_EFFECT("Tools", "Crop", "crop", CropEffect),

    DEFINE_EFFECT("Style", "Wave", "wave", WaveEffect),
    DEFINE_EFFECT("Style", "MattiasCRT", "mattiasCrt", CrtEffect),
    DEFINE_EFFECT("Style", "Drop Shadow", "dropShadow", DropShadowEffect),
    DEFINE_EFFECT("Style", "Long Shadow", "longShadow", LongShadowEffect),
    DEFINE_EFFECT("Style", "Pixelate", "pixelate", PixelateEffect),
    DEFINE_EFFECT("Style", "Threshold", "threshold", ThresholdEffect),
    DEFINE_EFFECT("Style", "Twist", "twist", TwistEffect),

    DEFINE_EFFECT("Generate", "Grid", "grid", GridEffect),
};
