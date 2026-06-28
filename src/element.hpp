#pragma once
#include <vector>

struct Rect {
    int x, y, w, h;
};

class Effect {};

class Element {
  public:
    ~Element();
    int x{0};
    int y{0};
    int w{100};
    int h{100};
    std::vector<Effect *> effects;

    Rect getBoundingBox();
    bool render(unsigned char *target);
};

class RectangleElement : public Element {
  public:
    RectangleElement();
    virtual ~RectangleElement() {}
};
