#include "variant.hpp"
#include <math.h>

Font Variant::defaultFont = {};

std::string getFontHash(const Font &font) {
    return font.path + ":" + std::to_string(font.index);
}

Brush::Type getBrushTypeFromString(const std::string &str) {
    if (str == "singleColor") {
        return Brush::Type::SingleColor;
    } else if (str == "linearGradient") {
        return Brush::Type::LinearGradient;
    } else if (str == "radialGradient") {
        return Brush::Type::RadialGradient;
    }
    return Brush::Type::SingleColor;
}

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
    } else if (type == "bool") {
        return VariantTypeEnum::Bool;
    } else if (type == "easing") {
        return VariantTypeEnum::Easing;
    } else if (type == "brush") {
        return VariantTypeEnum::Brush;
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
        return Variant(defaultFont);
    case VariantTypeEnum::Bool:
        return Variant(false);
    case VariantTypeEnum::Easing:
        return Variant(Easing{""});
    case VariantTypeEnum::Brush:
        return Variant(Brush{});
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
        lua_newtable(L);
        lua_pushstring(L, "i");
        lua_pushinteger(L, value.index);
        lua_settable(L, -3);
        lua_pushstring(L, "p");
        lua_pushstring(L, value.path.c_str());
        lua_settable(L, -3);
        break;
    }
    case VariantTypeEnum::Bool:
        lua_pushboolean(L, get<bool>());
        break;
    case VariantTypeEnum::Easing: {
        Easing value = get<Easing>();
        if (value.easingCurve.empty()) {
            lua_getglobal(L, "saturate");
        } else {
            lua_getglobal(L, value.easingCurve.c_str());
        }
        break;
    }
    case VariantTypeEnum::Brush: {
        Brush value = get<Brush>();
        switch (value.brushType) {
        case Brush::Type::SingleColor: {
            std::string code = "return function(x,y,w,h) return ";
            code += std::to_string(value.color1.r) + "," +
                    std::to_string(value.color1.g) + "," +
                    std::to_string(value.color1.b) + "," +
                    std::to_string(value.color1.a) + " end";
            luaL_dostring(L, code.c_str());
            break;
        }
        case Brush::Type::LinearGradient: {
            std::string code = "return function(x,y,w,h) local uvX = x / w - "
                               "0.5\nlocal uvY = y / h - 0.5\nreturn mixColor(";
            float rad = -value.angle * M_PI / 180.f;
            code += std::to_string(value.color1.r) + "," +
                    std::to_string(value.color1.g) + "," +
                    std::to_string(value.color1.b) + "," +
                    std::to_string(value.color1.a) + "," +
                    std::to_string(value.color2.r) + "," +
                    std::to_string(value.color2.g) + "," +
                    std::to_string(value.color2.b) + "," +
                    std::to_string(value.color2.a) + ",saturate(cos(" +
                    std::to_string(rad) +
                    " + atan2(uvY, uvX)) * length(uvX, uvY) + 0.5)) end";
            luaL_dostring(L, code.c_str());
            break;
        }
        case Brush::Type::RadialGradient: {
            std::string code = "return function(x,y,w,h) return mixColor(";
            code += std::to_string(value.color1.r) + "," +
                    std::to_string(value.color1.g) + "," +
                    std::to_string(value.color1.b) + "," +
                    std::to_string(value.color1.a) + "," +
                    std::to_string(value.color2.r) + "," +
                    std::to_string(value.color2.g) + "," +
                    std::to_string(value.color2.b) + "," +
                    std::to_string(value.color2.a) +
                    ",distance(x/w,y/h,0.5,0.5) * 2) end";
            luaL_dostring(L, code.c_str());
            break;
        }
        }
        break;
    }
    };
}

Variant Variant::getFromLua(VariantTypeEnum::Enum type, lua_State *L,
                            int index) {
    switch (type) {
    case VariantTypeEnum::None:
        return Variant(nullptr);
    case VariantTypeEnum::String: {
        const char *str = lua_tostring(L, index);
        return Variant(std::string(str));
    }
    case VariantTypeEnum::Int: {
        int value = lua_tointeger(L, index);
        return Variant(value);
    }
    case VariantTypeEnum::Double: {
        double value = lua_tonumber(L, index);
        return Variant(value);
    }
    case VariantTypeEnum::Vector2DInt: {
        lua_getfield(L, index, "x");
        int x = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, index, "y");
        int y = lua_tonumber(L, -1);
        lua_pop(L, 1);
        return Variant(Vector2DInt{x, y});
    }
    case VariantTypeEnum::Color: {
        lua_getfield(L, index, "r");
        int r = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, index, "g");
        int g = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, index, "b");
        int b = lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, index, "a");
        int a = 255;
        if (lua_isnumber(L, -1)) {
            a = lua_tonumber(L, -1);
        }
        lua_pop(L, 1);
        return Variant(Color{r, g, b, a});
    }
    case VariantTypeEnum::Font: {
        lua_getfield(L, index, "i");
        int fontIndex = lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, index, "p");
        const char *filePath = lua_tostring(L, -1);
        lua_pop(L, 1);
        return Variant(Font{filePath, fontIndex, ""});
    }
    case VariantTypeEnum::Bool: {
        bool value = lua_toboolean(L, index);
        return Variant(value);
    }
    case VariantTypeEnum::Easing: {
        const char *str = lua_tostring(L, index);
        return Variant(Easing{str});
    }
    case VariantTypeEnum::Brush: {
        Brush brush;

        lua_getfield(L, index, "type");
        const char *brushTypeString = lua_tostring(L, index);
        if (brushTypeString != nullptr) {
            brush.brushType = getBrushTypeFromString(brushTypeString);
        }
        lua_pop(L, 1);

        lua_getfield(L, index, "color1");
        if (lua_istable(L, -1)) {
            brush.color1 =
                getFromLua(VariantTypeEnum::Color, L, -1).get<Color>();
        }
        lua_pop(L, 1);

        lua_getfield(L, index, "color2");
        if (lua_istable(L, -1)) {
            brush.color2 =
                getFromLua(VariantTypeEnum::Color, L, -1).get<Color>();
        }
        lua_pop(L, 1);

        lua_getfield(L, index, "angle");
        brush.angle = lua_tonumber(L, -1);
        lua_pop(L, 1);

        return Variant(brush);
    }
    };
}
