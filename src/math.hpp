#pragma once
#include <cmath>

static inline float mix(float t, float min, float max) {
    return (t * (max - min)) + min;
}

static double linear(double x) { return x; }

static double easeInQuad(double x) { return x * x; }

static double easeOutQuad(double x) { return 1 - (1 - x) * (1 - x); }

static double easeInOutQuad(double x) {
    if (x < 0.5) {
        return 2 * x * x;
    } else {
        return 1 - std::pow((-2 * x + 2), 2) / 2;
    }
}

static double easeInCubic(double x) { return x * x * x; }

static double easeOutCubic(double x) { return 1 - std::pow((1 - x), 3); }

static double easeInOutCubic(double x) {
    if (x < 0.5) {
        return 4 * x * x * x;
    } else {
        return 1 - std::pow((-2 * x + 2), 3) / 2;
    }
}

static double easeInQuart(double x) { return x * x * x * x; }

static double easeOutQuart(double x) { return 1 - std::pow((1 - x), 4); }

static double easeInOutQuart(double x) {
    if (x < 0.5) {
        return 8 * x * x * x * x;
    } else {
        return 1 - std::pow((-2 * x + 2), 4) / 2;
    }
}

static double easeInQuint(double x) { return x * x * x * x * x; }

static double easeOutQuint(double x) { return 1 - std::pow((1 - x), 5); }

static double easeInOutQuint(double x) {
    if (x < 0.5) {
        return 16 * x * x * x * x * x;
    } else {
        return 1 - std::pow((-2 * x + 2), 5) / 2;
    }
}

static double easeInSine(double x) { return 1 - std::cos((x * M_PI) / 2); }

static double easeOutSine(double x) { return std::sin((x * M_PI) / 2); }

static double easeInOutSine(double x) { return -(std::cos(M_PI * x) - 1) / 2; }

static double easeInExpo(double x) {
    if (x == 0) {
        return x;
    }
    return std::pow(2, (10 * x - 10));
}

static double easeOutExpo(double x) {
    if (x == 1) {
        return x;
    }
    return 1 - std::pow(2, (-10 * x));
}

static double easeInOutExpo(double x) {
    if (x == 0 || x == 1) {
        return x;
    }

    if (x < 0.5) {
        return std::pow(2, (20 * x - 10)) / 2;
    } else {
        return (2 - std::pow(2, (-20 * x + 10))) / 2;
    }
}

static double easeInCirc(double x) { return 1 - sqrt(1 - std::pow(x, 2)); }

static double easeOutCirc(double x) { return sqrt(1 - std::pow((x - 1), 2)); }

static double easeInOutCirc(double x) {
    if (x < 0.5) {
        return (1 - sqrt(1 - std::pow((2 * x), 2))) / 2;
    } else {
        return (sqrt(1 - std::pow((-2 * x + 2), 2)) + 1) / 2;
    }
}

static double easeInBack(double x) {
    return 2.70158 * x * x * x - 1.70158 * x * x;
}

static double easeOutBack(double x) {
    return 1 + 2.70158 * std::pow((x - 1), 3) + 1.70158 * std::pow((x - 1), 2);
}

static double easeInOutBack(double x) {
    if (x < 0.5) {
        return (std::pow((2 * x), 2) * ((2.5949095 + 1) * 2 * x - 2.5949095)) /
               2;
    } else {
        return (std::pow((2 * x - 2), 2) *
                    ((2.5949095 + 1) * (x * 2 - 2) + 2.5949095) +
                2) /
               2;
    }
}

static double easeInElastic(double x) {
    if (x == 0 || x == 1) {
        return x;
    }
    return -std::pow(2, (10 * x - 10)) *
           std::sin((x * 10 - 10.75) * 2.0943951023931953);
}

static double easeOutElastic(double x) {
    if (x == 0 || x == 1) {
        return x;
    }
    return std::pow(2, (-10 * x)) *
               std::sin((x * 10 - 0.75) * 2.0943951023931953) +
           1;
}

static double easeInOutElastic(double x) {
    if (x == 0 || x == 1) {
        return x;
    }
    if (x < 0.5) {
        return -(std::pow(2, (20 * x - 10)) *
                 std::sin((20 * x - 11.125) * 1.3962634015954636)) /
               2;
    } else {
        return (std::pow(2, (-20 * x + 10)) *
                std::sin((20 * x - 11.125) * 1.3962634015954636)) /
                   2 +
               1;
    }
}

static double easeOutBounce(double x) {
    double n1 = 7.5625;
    double d1 = 2.75;

    if (x < 1 / d1) {
        return n1 * x * x;
    } else if (x < 2 / d1) {
        x = x - 1.5 / d1;
        return n1 * x * x + 0.75;
    } else if (x < 2.5 / d1) {
        x = x - 2.25 / d1;
        return n1 * x * x + 0.9375;
    } else {
        x = x - 2.625 / d1;
        return n1 * x * x + 0.984375;
    }
}

static double easeInBounce(double x) { return 1 - easeOutBounce(1 - x); }

static double easeInOutBounce(double x) {
    if (x < 0.5) {
        return (1 - easeOutBounce(1 - 2 * x)) / 2;
    } else {
        return (1 + easeOutBounce(2 * x - 1)) / 2;
    }
}
