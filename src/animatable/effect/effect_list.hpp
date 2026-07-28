#pragma once
#include "effect.hpp"
#include <QString>

#define DEFINE_EFFECT(category, displayName, name, type)                       \
    EffectInfo {                                                               \
        QStringLiteral(category), QStringLiteral(displayName),                 \
            QStringLiteral(name), []() { return new type(); }                  \
    }

struct EffectInfo {
    QString category;
    QString displayName;
    QString name;
    std::function<Effect *()> create;

    QString sortString() const {
        return (category + ":" + displayName).toLower();
    }
};

extern std::vector<EffectInfo> effectList;
