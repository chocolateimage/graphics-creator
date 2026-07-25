#include "threshold_effect.hpp"
#include "math.hpp"

ThresholdEffect::ThresholdEffect() {
    channel.enumList.push_back("Luminance");
    channel.enumList.push_back("RGB");
    channel.enumList.push_back("Alpha");
    channel.setMin(0);
    channel.setMax(channel.enumList.size() - 1);
    threshold.setMin(0);
    threshold.setMax(255);
}

bool ThresholdEffectRender::render(const uint32_t *source,
                                   const Rect &sourceRect, uint32_t *target) {
    Rect rect = renderBox;
    int t = this->threshold;
    int channel = this->channel;
    for (int y = 0; y < sourceRect.h; y++) {
        for (int x = 0; x < sourceRect.w; x++) {
            auto [r, g, b, a] =
                extractRGBA(source[pixelIndex(x, y, sourceRect.w)]);
            if (channel == 0) {
                int lum = (r + g + b) / 3;
                if (lum >= t) {
                    r = 255;
                    g = 255;
                    b = 255;
                } else {
                    r = 0;
                    g = 0;
                    b = 0;
                }
            } else if (channel == 1) {
                if (r >= t) {
                    r = 255;
                } else {
                    r = 0;
                }
                if (g >= t) {
                    g = 255;
                } else {
                    g = 0;
                }
                if (b >= t) {
                    b = 255;
                } else {
                    b = 0;
                }
            } else if (channel == 2) {
                if (a >= t) {
                    a = 255;
                } else {
                    a = 0;
                }
            }
            target[pixelIndex(x, y, rect.w)] = makePixel(r, g, b, a);
        }
    }
    return true;
}
