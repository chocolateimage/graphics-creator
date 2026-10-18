#include "ansi_suite.hpp"

int customSnprintf(char *buf, size_t size, const char *format, ...) {
    // qInfo() << "CALL custom Snprintf:" << format;
    va_list args;
    va_start(args, format);
    vsnprintf(buf, size, format, args);
    va_end(args);
    return 0;
}

PFAnsiSuite2::PFAnsiSuite2() {
    atan = ::atan;
    atan2 = ::atan2;
    ceil = ::ceil;
    cos = ::cos;
    exp = ::exp;
    fabs = ::fabs;
    floor = ::floor;
    fmod = ::fmod;
    hypot = ::hypot;
    log = ::log;
    log10 = ::log10;
    pow = ::pow;
    sin = ::sin;
    sqrt = ::sqrt;
    tan = ::tan;
    sprintf = ::sprintf;
    strcpy = ::strcpy;
    asin = ::asin;
    acos = ::acos;

    snprintf = customSnprintf;
}
