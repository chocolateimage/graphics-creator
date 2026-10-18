#pragma once
#include "ae/ae_base.hpp"

struct PFAnsiSuite2 {
    PFAnsiSuite2();

    double (*atan)(double);
    double (*atan2)(double, double);
    double (*ceil)(double);
    double (*cos)(double);
    double (*exp)(double);
    double (*fabs)(double);
    double (*floor)(double);
    double (*fmod)(double, double);
    double (*hypot)(double, double);
    double (*log)(double);
    double (*log10)(double);
    double (*pow)(double, double);
    double (*sin)(double);
    double (*sqrt)(double);
    double (*tan)(double);
    int (*sprintf)(char *, const char *, ...);
    char *(*strcpy)(char *, const char *);
    double (*asin)(double);
    double (*acos)(double);
    void *func20{nullptr};
    int (*snprintf)(char *, size_t, const char *, ...);
};