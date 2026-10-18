#include "fill_matte_suite.hpp"
#include "ae/utility_callbacks.hpp"

PFFillMatteSuite2::PFFillMatteSuite2() {
    fill = ::fill;
    fill16 = ::fill16;
    fillFloat = ::fillFloat;
    premultiply = ::premultiply;
    premultiplyColor = ::premultiplyColor;
    premultiplyColor16 = ::premultiplyColor16;
    premultiplyColorFloat = ::premultiplyColorFloat;
}
