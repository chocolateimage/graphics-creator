#pragma once
#include "animatable/animatable.hpp"
#include "animatable/effect/effect.hpp"
#include "animatable/property.hpp"
#include "variant.hpp"
#include <QObject>
#include <QRect>
#include <string>

class RenderThread;

static constexpr uint32_t makePixel(Color color) {
    return (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
}

class Element : public Animatable {
    Q_OBJECT
  public:
    Element() {
        id = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
        w.setMin(1);
        h.setMin(1);
        w.setMax(5000);
        h.setMax(5000);
    };
    ~Element();
    QString id;
    Property<int> x{this, "x", 0};
    Property<int> y{this, "y", 0};
    Property<int> w{this, "w", 100};
    Property<int> h{this, "h", 100};
    int startFrame{0};
    int durationFrames{INT32_MIN};
    bool collapsed{true};
    // Text editing
    bool editMode{false};
    QList<Effect *> effects;
    bool visible{true};

    AnimatableRender *toRender(const FrameInfo &frameInfo) override;
    virtual QRect getBoundingBox(const FrameInfo &frameInfo);
    QJsonObject serialize() override;
    void deserialize(const QJsonObject &obj) override;
    void addEffect(Effect *effect);
    void insertEffect(Effect *effect, int index);
    void removeEffect(Effect *effect);
    void reorderEffect(Effect *effect, int newIndex);

    void setEditMode(bool newMode);
    void setVisible(bool visible);

    virtual QString const typeName() = 0;

  signals:
    void effectAdded(Effect *effect, int index);
    void effectRemoved(Effect *effect);
    void effectListUpdated();
    void effectPropertyUpdated(Effect *effect, PropertyBase *property);
    void editModeUpdated(bool newMode);
    void visibilityUpdated(bool newValue);
};

class ElementRender : public AnimatableRender {
    Q_OBJECT
  public:
    ElementRender() {};
    ~ElementRender();

    QString id;
    PropertyRender<int> x{this};
    PropertyRender<int> y{this};
    PropertyRender<int> w{this};
    PropertyRender<int> h{this};
    std::vector<EffectRender *> effects;
    int startFrame;
    int durationFrames;

    RenderThread *renderThread{nullptr};

    virtual void prepare() {};
    virtual Rect getRenderBox();
    virtual bool render(uint32_t *target) = 0;

    bool visible;

    int currentFrame{0};
    double currentSeconds{0};
};
