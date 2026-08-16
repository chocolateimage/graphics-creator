#include "video_element.hpp"
#include "math.hpp"
#include "render.hpp"

VideoElement::VideoElement() : Element() {
    scaleType.enumList.push_back("Nearest neighbour");
    scaleType.enumList.push_back("Bilinear");
    scaleType.enumList.push_back("Bicubic");
    scaleType.enumList.push_back("Area");
    scaleType.enumList.push_back("Gaussian");
    scaleType.enumList.push_back("Sinc");
    scaleType.enumList.push_back("Lanczos");
    scaleType.enumList.push_back("Spline");
    scaleType.updateBoundsToEnumList();
}

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
    int scaleFlags = SWS_POINT;
    int scaleType = this->scaleType;
    if (scaleType == 0) {
        scaleFlags = SWS_POINT;
    } else if (scaleType == 1) {
        scaleFlags = SWS_BILINEAR;
    } else if (scaleType == 2) {
        scaleFlags = SWS_BICUBIC;
    } else if (scaleType == 3) {
        scaleFlags = SWS_AREA;
    } else if (scaleType == 4) {
        scaleFlags = SWS_GAUSS;
    } else if (scaleType == 5) {
        scaleFlags = SWS_SINC;
    } else if (scaleType == 6) {
        scaleFlags = SWS_LANCZOS;
    } else if (scaleType == 7) {
        scaleFlags = SWS_SPLINE;
    }
    AVFrame *frame = videoData->getFrame(currentSeconds, w, h, scaleFlags);
    if (!frame) {
        return true;
    }
    for (int y = 0; y < h; y++) {
        memcpy(target + y * w, frame->data[0] + y * frame->linesize[0], w * 4);
    }
    av_frame_free(&frame);

    return true;
}
