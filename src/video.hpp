#pragma once

extern "C" {
#include <libavutil/frame.h>
}

class Video {
  public:
    int width;
    int height;
    float frameRate;
    float duration;

    AVFrame *allocateFrame();
};
