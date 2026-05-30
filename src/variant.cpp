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
    } else if (type == "font") {
        return VariantTypeEnum::Font;
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
        return Variant(Color{0, 0, 0, 255});
    case VariantTypeEnum::Vector2DInt:
        return Variant(Vector2DInt{0, 0});
    case VariantTypeEnum::Font:
        return Variant(Font{});
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
    case VariantTypeEnum::Vector2DInt: {
        Vector2DInt value = get<Vector2DInt>();
        lua_newtable(L);
        lua_pushstring(L, "x");
        lua_pushnumber(L, value.x);
        lua_settable(L, -3);
        lua_pushstring(L, "y");
        lua_pushnumber(L, value.y);
        lua_settable(L, -3);
        break;
    }
    case VariantTypeEnum::Color: {
        Color value = get<Color>();
        lua_newtable(L);
        lua_pushstring(L, "r");
        lua_pushnumber(L, value.r);
        lua_settable(L, -3);
        lua_pushstring(L, "g");
        lua_pushnumber(L, value.g);
        lua_settable(L, -3);
        lua_pushstring(L, "b");
        lua_pushnumber(L, value.b);
        lua_settable(L, -3);
        lua_pushstring(L, "a");
        lua_pushnumber(L, value.a);
        lua_settable(L, -3);
        break;
    }
    case VariantTypeEnum::Font: {
        Font value = get<Font>();
        lua_pushlightuserdata(L, &value);
        break;
    }
    };
}
