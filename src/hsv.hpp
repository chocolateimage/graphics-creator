#pragma once
#include <array>
#include <cmath>

// Source - https://stackoverflow.com/a/6930407
// Posted by David Hoerl, modified by community. See post 'Timeline' for change
// history Retrieved 2026-07-26, License - CC BY-SA 3.0

struct HSV {
    double h; // angle in degrees
    double s; // a fraction between 0 and 1
    double v; // a fraction between 0 and 1
};

static HSV rgb2hsv(double inR, double inG, double inB) {
    HSV out;
    double min, max, delta;

    min = inR < inG ? inR : inG;
    min = min < inB ? min : inB;

    max = inR > inG ? inR : inG;
    max = max > inB ? max : inB;

    out.v = max; // v
    delta = max - min;
    if (delta < 0.00001) {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max); // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN; // its now undefined
        return out;
    }
    if (inR >= max)                  // > is bogus, just keeps compilor happy
        out.h = (inG - inB) / delta; // between yellow & magenta
    else if (inG >= max)
        out.h = 2.0 + (inB - inR) / delta; // between cyan & yellow
    else
        out.h = 4.0 + (inR - inG) / delta; // between magenta & cyan

    out.h *= 60.0; // degrees

    if (out.h < 0.0)
        out.h += 360.0;

    return out;
}

static std::array<double, 3> hsv2rgb(const HSV &in) {
    double hh, p, q, t, ff;
    long i;

    if (in.s <= 0.0) { // < is bogus, just shuts up warnings
        return {in.v, in.v, in.v};
    }
    hh = in.h;
    if (hh >= 360.0)
        hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch (i) {
    case 0:
        return {in.v, t, p};
    case 1:
        return {q, in.v, p};
    case 2:
        return {p, in.v, t};

    case 3:
        return {p, q, in.v};
    case 4:
        return {t, p, in.v};
    case 5:
    default:
        return {in.v, p, q};
    }
}
