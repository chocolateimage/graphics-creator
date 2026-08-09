#pragma once
#include "element.hpp"

class GroupElement : public Element {
  public:
    GroupElement();
    virtual ~GroupElement() {}

    AnimatableRender *createClass() override;
    QRect getRawBoundingBox(const FrameInfo &frameInfo) override;
    bool isResizable() const override { return false; }

    void ungroup();

    QList<Element *> getChildren() const;
    bool isDirectChild(Element *element) const;
    bool isAnyChild(Element *element) const;

    QString const typeName() override { return "group"; }
};

class GroupElementRender : public ElementRender {
  public:
    GroupElementRender() : ElementRender() {}
    virtual ~GroupElementRender() {}

    QList<ElementRender *> children;

    void prepare() override;
    Rect getRenderBox() override;
    bool render(uint32_t *target) override;
};
