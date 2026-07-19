#pragma once
#include "effect.hpp"
#include <QString>

#define DEFINE_EFFECT(category, displayName, name, type)                       \
    EffectInfo {                                                               \
        QStringLiteral(category), QStringLiteral(displayName),                 \
            QStringLiteral(name), []() { return new type(); }                  \
    }

struct EffectInfo {
    const QString category;
    const QString displayName;
    const QString name;
    const std::function<Effect *()> create;
};

extern const std::vector<EffectInfo> effectList;
