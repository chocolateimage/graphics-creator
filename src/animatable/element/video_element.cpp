#include "video_element.hpp"
#include "math.hpp"
#include "render.hpp"

VideoElement::VideoElement() : Element() {}

AnimatableRender *VideoElement::createClass() {
    return new VideoElementRender();
}

bool VideoElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    int w = this->w;
    int h = this->h;

    auto videoData = globalLoader.loadVideo(this->path);
    if (videoData->error) {
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                target[pixelIndex(x, y, w)] = makePixel(240, 50, 50, 255);
            }
        }
        return true;
    }
    AVFrame *frame = videoData->getFrame(currentSeconds, w, h);
    if (!frame) {
        return true;
    }
    for (int y = 0; y < h; y++) {
        memcpy(target + y * w, frame->data[0] + y * frame->linesize[0], w * 4);
    }
    av_frame_free(&frame);

    return true;
}
