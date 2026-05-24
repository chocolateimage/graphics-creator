#include "variant.hpp"

Variant::Variant(VariantType variant) : m_variant(variant) {}

Variant::Variant(const Variant &variant) {
    m_variant = variant.m_variant;
    // TODO: deep copy structs.. maybe... is it necessary?
}

VariantType Variant::variant() const { return m_variant; }

VariantTypeEnum::Enum Variant::type() const {
    return (VariantTypeEnum::Enum)m_variant.index();
}

VariantTypeEnum::Enum Variant::typeFromString(const std::string &type) {
    if (type == "none") {
        return VariantTypeEnum::None;
    } else if (type == "string") {
        return VariantTypeEnum::String;
    } else if (type == "int") {
        return VariantTypeEnum::Int;
    } else if (type == "double") {
        return VariantTypeEnum::Double;
    } else if (type == "color") {
        return VariantTypeEnum::Color;
    } else if (type == "vector2dint") {
        return VariantTypeEnum::Vector2DInt;
    } else {
        return (VariantTypeEnum::Enum)-1;
    }
}

Variant Variant::getDefault(VariantTypeEnum::Enum type) {
    switch (type) {
    case VariantTypeEnum::None:
        return Variant(nullptr);
    case VariantTypeEnum::String:
        return Variant("");
    case VariantTypeEnum::Int:
        return Variant(0);
    case VariantTypeEnum::Double:
        return Variant(0.0);
    case VariantTypeEnum::Color:
        return Variant((Color){0, 0, 0});
    case VariantTypeEnum::Vector2DInt:
        return Variant((Vector2DInt){0, 0});
    }
}

bool Variant::isValidType(const std::string &type) {
    return (int)typeFromString(type) != -1;
}

void Variant::pushLua(lua_State *L) const {
    switch (type()) {
    case VariantTypeEnum::None:
        lua_pushnil(L);
        break;
    case VariantTypeEnum::String:
        lua_pushstring(L, get<std::string>().c_str());
        break;
    case VariantTypeEnum::Int:
        lua_pushnumber(L, get<int>());
        break;
    case VariantTypeEnum::Double:
        lua_pushnumber(L, get<double>());
        break;
    case VariantTypeEnum::Color: // TODO: implement
    case VariantTypeEnum::Vector2DInt:
        lua_pushnil(L);
        break;
    };
}
