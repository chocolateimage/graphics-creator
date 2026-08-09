#include "group_element.hpp"
#include "math.hpp"
#include "render.hpp"
#include "scene.hpp"

GroupElement::GroupElement() : Element() {
    w.hidden = true;
    h.hidden = true;
}

QRect GroupElement::getRawBoundingBox(const FrameInfo &frameInfo) {
    QList<Element *> children = getChildren();
    QRect rect;
    for (auto element : children) {
        QRect childBox = element->getRawBoundingBox(frameInfo);
        if (rect.isNull()) {
            rect = childBox;
        } else {
            rect = rect.united(childBox);
        }
    }
    rect.translate(x.get(frameInfo), y.get(frameInfo));
    return rect;
}

AnimatableRender *GroupElement::createClass() {
    return new GroupElementRender();
}

QList<Element *> GroupElement::getChildren() const {
    QList<Element *> children;
    if (scene) {
        for (auto element : scene->elements) {
            if (element->getParent() == id) {
                children.append(element);
            }
        }
    }
    return children;
}

bool GroupElement::isDirectChild(Element *element) const {
    for (auto child : getChildren()) {
        if (child == element) {
            return true;
        }
    }

    return false;
}

bool GroupElement::isAnyChild(Element *element) const {
    for (auto child : getChildren()) {
        if (child == element) {
            return true;
        }

        GroupElement *groupElement = dynamic_cast<GroupElement *>(child);
        if (groupElement && groupElement->isAnyChild(element)) {
            return true;
        }
    }

    return false;
}

void GroupElement::ungroup() {
    QList<Element *> children = getChildren();
    for (auto element : children) {
        element->setParent(getParent());
    }

    scene->selectElements(children);
    scene->removeElement(this);
    delete this; // TODO: undo redo so not this
}

void GroupElementRender::prepare() {
    for (auto element : renderThread->currentFrameTask->renderElements) {
        if (element->getParent() == id) {
            children.append(element);
        }
    }
}

Rect GroupElementRender::getRenderBox() {
    QRect rect;
    for (auto element : children) {
        QRect childBox = element->getRenderBox().toQRect();
        if (rect.isNull()) {
            rect = childBox;
        } else {
            rect = rect.united(childBox);
        }
    }
    rect.translate(x.get(), y.get());
    return Rect::fromQRect(rect);
}

bool GroupElementRender::render(uint32_t *target) {
    auto rect = getRenderBox();
    int w = this->w;
    int h = this->h;
    int ox = rect.x - x.get();
    int oy = rect.y - y.get();

    for (auto child : children) {
        if (!child->visible)
            continue;

        if (currentFrame < child->startFrame ||
            currentFrame >= child->startFrame + child->durationFrames) {
            continue;
        }

        ElementSelection selection;
        selection.elementId = child->id;
        selection.frameType = ElementSelection::FrameType::Final;

        ElementSelectionSnippet snippet = renderThread->getSnippet(selection);
        for (int y = 0; y < snippet.rect.h; y++) {
            for (int x = 0; x < snippet.rect.w; x++) {
                int sx = x + snippet.rect.x - ox;
                int sy = y + snippet.rect.y - oy;
                if (sx < 0 || sy < 0 || sx >= rect.w || sy >= rect.h)
                    continue;
                auto index = pixelIndex(sx, sy, rect.w);
                target[index] =
                    over(target[index],
                         snippet.values[pixelIndex(x, y, snippet.rect.w)]);
            }
        }
    }

    // for (int y = 0; y < h; y++) {
    //     for (int x = 0; x < w; x++) {
    //         target[pixelIndex(x, y, rect.w)] = makePixel(255, 0, 0, 255);
    //     }
    // }
    return true;
}
