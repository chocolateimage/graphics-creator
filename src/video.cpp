#include "video.hpp"

AVFrame *Video::allocateFrame() {
    AVFrame *frame = av_frame_alloc();
    frame->width = width;
    frame->height = height;
    return frame;
}
