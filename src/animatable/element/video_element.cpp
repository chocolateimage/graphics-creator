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
    AVFrame *frame = videoData->getFrame(currentFrame, w, h);
    for (int y = 0; y < h; y++) {
        memcpy(target + y * w, frame->data[0] + y * frame->linesize[0], w * 4);
    }
    av_frame_free(&frame);

    return true;
}
