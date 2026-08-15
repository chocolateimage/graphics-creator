#include "effect_list.hpp"
#include "blur_effect.hpp"
#include "brightness_effect.hpp"
#include "color_link_effect.hpp"
#include "corner_pin_effect.hpp"
#include "crop_effect.hpp"
#include "crt_effect.hpp"
#include "drop_shadow_effect.hpp"
#include "expand_render_box_effect.hpp"
#include "fill_effect.hpp"
#include "flip_effect.hpp"
#include "grid_effect.hpp"
#include "hsv_effect.hpp"
#include "in_out_effect.hpp"
#include "invert_effect.hpp"
#include "levels_effect.hpp"
#include "long_shadow_effect.hpp"
#include "matte_effect.hpp"
#include "noise_effect.hpp"
#include "offset_effect.hpp"
#include "opacity_effect.hpp"
#include "pixelate_effect.hpp"
#include "place_original_effect.hpp"
#include "rotation_effect.hpp"
#include "scale_effect.hpp"
#include "threshold_effect.hpp"
#include "tile_effect.hpp"
#include "tint_effect.hpp"
#include "twist_effect.hpp"
#include "vignette_effect.hpp"
#include "wave_effect.hpp"

std::vector<EffectInfo> effectList = {
    DEFINE_EFFECT("Blur", "Box Blur", "boxBlur", BlurEffect),

    DEFINE_EFFECT("Color", "Opacity", "opacity", OpacityEffect),
    DEFINE_EFFECT("Color", "Brightness", "brightness", BrightnessEffect),
    DEFINE_EFFECT("Color", "Tint", "tint", TintEffect),
    DEFINE_EFFECT("Color", "Invert", "invert", InvertEffect),
    DEFINE_EFFECT("Color", "Hue/Saturation", "hsv", HsvEffect),
    DEFINE_EFFECT("Color", "Fill", "fill", FillEffect),
    DEFINE_EFFECT("Color", "Color Link", "colorLink", ColorLinkEffect),
    DEFINE_EFFECT("Color", "Levels", "levels", LevelsEffect),

    DEFINE_EFFECT("Tools", "Offset", "offset", OffsetEffect),
    DEFINE_EFFECT("Tools", "Scale", "scale", ScaleEffect),
    DEFINE_EFFECT("Tools", "Flip", "flip", FlipEffect),
    DEFINE_EFFECT("Tools", "Place Original", "placeOriginal",
                  PlaceOriginalEffect),
    DEFINE_EFFECT("Tools", "Crop", "crop", CropEffect),
    DEFINE_EFFECT("Tools", "In/Out", "inOut", InOutEffect),
    DEFINE_EFFECT("Tools", "Matte", "matte", MatteEffect),
    DEFINE_EFFECT("Tools", "Expand Render Box", "expandRenderBox",
                  ExpandRenderBoxEffect),
    DEFINE_EFFECT("Tools", "Rotation", "rotation", RotationEffect),

    DEFINE_EFFECT("Style", "Wave", "wave", WaveEffect),
    DEFINE_EFFECT("Style", "MattiasCRT", "mattiasCrt", CrtEffect),
    DEFINE_EFFECT("Style", "Drop Shadow", "dropShadow", DropShadowEffect),
    DEFINE_EFFECT("Style", "Long Shadow", "longShadow", LongShadowEffect),
    DEFINE_EFFECT("Style", "Pixelate", "pixelate", PixelateEffect),
    DEFINE_EFFECT("Style", "Threshold", "threshold", ThresholdEffect),
    DEFINE_EFFECT("Style", "Twist", "twist", TwistEffect),
    DEFINE_EFFECT("Style", "Corner Pin", "cornerPin", CornerPinEffect),
    DEFINE_EFFECT("Style", "Tile", "tile", TileEffect),
    DEFINE_EFFECT("Style", "Vignette", "vignette", VignetteEffect),

    DEFINE_EFFECT("Generate", "Grid", "grid", GridEffect),
    DEFINE_EFFECT("Generate", "Noise", "noise", NoiseEffect),
};
