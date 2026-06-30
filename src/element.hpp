#pragma once
#include <QObject>
#include <string>
#include <vector>

static constexpr uint32_t makePixel(uint8_t red, uint8_t green, uint8_t blue,
                                    uint8_t alpha) {
    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

static constexpr int pixelIndex(int x, int y, int stride) {
    return y * stride + x;
}

struct Rect {
    int x, y, w, h;
};

class Effect {};

class Element : public QObject {
    Q_OBJECT
  public:
    Element() {};
    ~Element();
    int x{0};
    int y{0};
    int w{100};
    int h{100};
    std::string name;
    std::vector<Effect *> effects;

    Rect getRenderBox();
    virtual bool render(uint32_t *target) = 0;

    // TODO: "properties" that are named with strings so you can animate them
    // ("x", "y", "fill", whatever)
};

class RectangleElement : public Element {
  public:
    RectangleElement();
    virtual ~RectangleElement() {}

    virtual bool render(uint32_t *target);
};
