#pragma once

#include "lua.hpp"
#include <string>
#include <variant>

/*
what should it store:

- strings
- int
- double
- special font object with font path/font name, font weight/font settings (so
C++ classes)
- colors (R,G,B)
- Vector2Ds (point selected on the screen probably)
- Vector3Ds

---

it should:

- be able to be serialized and deserialized (JSON)
- probably have a type field to identify it
- be able to be controlled from Qt land

---

std::variant for storing?

*/

struct Color {
    int r, g, b, a;
};

struct Vector2DInt {
    int x, y;
};

using VariantType =
    std::variant<std::monostate, std::string, int, double, Color, Vector2DInt>;

struct VariantTypeEnum {
    enum Enum {
        None,
        String,
        Int,
        Double,
        Color,
        Vector2DInt,
    };
};

class Variant {
  public:
    Variant(VariantType variant);
    Variant(const Variant &variant);
    Variant &operator=(const Variant &) = default;

    VariantType variant() const;
    VariantTypeEnum::Enum type() const;

    template <typename T> T get() const { return std::get<T>(m_variant); }

    void pushLua(lua_State *L) const;

    static VariantTypeEnum::Enum typeFromString(const std::string &type);
    static Variant getDefault(VariantTypeEnum::Enum type);
    static bool isValidType(const std::string &type);

  private:
    VariantType m_variant;
};
