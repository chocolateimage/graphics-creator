static const char *LUA_BASIC_CODE = R"(
    function options() return {} end

    function draw(_frame)

    end
)";

static const char *LUA_GLOBAL_CODE = R"(
bit = require("bit")
ffi = require("ffi")
bor = bit.bor
lshift = bit.lshift
rshift = bit.rshift
band = bit.band
bor = bit.bor
bxor = bit.bxor
bnot = bit.bnot
abs = math.abs
acos = math.acos
asin = math.asin
atan = math.atan
atan2 = math.atan2
ceil = math.ceil
cos = math.cos
cosh = math.cosh
deg = math.deg
exp = math.exp
floor = math.floor
fmod = math.fmod
frexp = math.frexp
ldexp = math.ldexp
log = math.log
max = math.max
min = math.min
modf = math.modf
pow = math.pow
rad = math.rad
sin = math.sin
sinh = math.sinh
sqrt = math.sqrt
tan = math.tan
tanh = math.tanh
pi = math.pi
PI = pi
huge = math.huge

function easeInQuad(x)
    return x * x
end

function easeOutQuad(x)
    return 1 - (1 - x) * (1 - x)
end

function easeInOutQuad(x)
    if x < 0.5 then
        return 2 * x * x
    else
        return 1 - ((-2 * x + 2) ^ 2) / 2
    end
end

function easeInCubic(x)
    return x * x * x
end

function easeOutCubic(x)
    return 1 - ((1 - x) ^ 3)
end

function easeInOutCubic(x)
    if x < 0.5 then
        return 4 * x * x * x
    else
        return 1 - ((-2 * x + 2) ^ 3) / 2
    end
end

function easeInQuart(x)
    return x * x * x * x
end

function easeOutQuart(x)
    return 1 - ((1 - x) ^ 4)
end

function easeInOutQuart(x)
    if x < 0.5 then
        return 8 * x * x * x * x
    else
        return 1 - ((-2 * x + 2) ^ 4) / 2
    end
end

function easeInQuint(x)
    return x * x * x * x * x
end

function easeOutQuint(x)
    return 1 - ((1 - x) ^ 5)
end

function easeInOutQuint(x)
    if x < 0.5 then
        return 16 * x * x * x * x * x
    else
        return 1 - ((-2 * x + 2) ^ 5) / 2
    end
end

function easeInSine(x)
    return 1 - cos((x * PI) / 2)
end

function easeOutSine(x)
    return sin((x * PI) / 2)
end

function easeInOutSine(x)
    return -(cos(PI * x) - 1) / 2
end

function easeInExpo(x)
    if x == 0 then
        return x
    end
    return (2 ^ (10 * x - 10))
end

function easeOutExpo(x)
    if x == 1 then
        return x
    end
    return 1 - (2 ^ (-10 * x))
end

function easeInOutExpo(x)
    if x == 0 or x == 1 then
        return x
    end
    if x < 0.5 then
        return (2 ^ (20 * x - 10)) / 2
    else
        return (2 - (2 ^ (-20 * x + 10))) / 2
    end
end

function easeInCirc(x)
    return 1 - sqrt(1 - (x ^ 2))
end

function easeOutCirc(x)
    return sqrt(1 - ((x - 1) ^ 2))
end

function easeInOutCirc(x)
    if x < 0.5 then
        return (1 - sqrt(1 - ((2 * x) ^ 2))) / 2
    else
        return (sqrt(1 - ((-2 * x + 2) ^ 2)) + 1) / 2
    end
end

function easeInBack(x)
    return 2.70158 * x * x * x - 1.70158 * x * x
end

function easeOutBack(x)
    return 1 + 2.70158 * ((x - 1) ^ 3) + 1.70158 * ((x - 1) ^ 2)
end

function easeInOutBack(x)
    if x < 0.5 then
        return (((2 * x) ^ 2) * ((2.5949095 + 1) * 2 * x - 2.5949095)) / 2
    else
        return (((2 * x - 2) ^ 2) * ((2.5949095 + 1) * (x * 2 - 2) + 2.5949095) + 2) / 2
    end
end

function easeInElastic(x)
    if x == 0 or x == 1 then
        return x
    end
    return -(2 ^ (10 * x - 10)) * sin((x * 10 - 10.75) * 2.0943951023931953)
end

function easeOutElastic(x)
    if x == 0 or x == 1 then
        return x
    end
    return (2 ^ (-10 * x)) * sin((x * 10 - 0.75) * 2.0943951023931953) + 1
end

function easeInOutElastic(x)
    if x == 0 or x == 1 then
        return x
    end
    if x < 0.5 then
        return -((2 ^ (20 * x - 10)) * sin((20 * x - 11.125) * 1.3962634015954636)) / 2
    else
        return ((2 ^ (-20 * x + 10)) * sin((20 * x - 11.125) * 1.3962634015954636)) / 2 + 1
    end
end

function easeOutBounce(x)
    local n1 = 7.5625
    local d1 = 2.75

    if x < 1 / d1 then
        return n1 * x * x
    elseif x < 2 / d1 then
        x = x - 1.5 / d1
        return n1 * x * x + 0.75
    elseif x < 2.5 / d1 then
        x = x - 2.25 / d1
        return n1 * x * x + 0.9375
    else
        x = x - 2.625 / d1
        return n1 * x * x + 0.984375
    end
end

function easeInBounce(x)
    return 1 - easeOutBounce(1 - x)
end

function easeInOutBounce(x)
    if x < 0.5 then
        return (1 - easeOutBounce(1 - 2 * x)) / 2
    else
        return (1 + easeOutBounce(2 * x - 1)) / 2
    end
end

function clamp(x, from, to)
    if x < from then
        return from
    end
    if x > to then
        return to
    end
    return x
end

function saturate(x)
    return min(max(x, 0), 1)
end

function mix(from, to, x)
    return (x * (to - from)) + from
end

lerp = mix

function linearstep(from, to, x)
    return saturate((x - from) / (to - from))
end

function smoothstep(from, to, x)
    x = saturate((x - from) / (to - from))
    return x * x * (3 - 2 * x)
end

function fract(x)
    return x - floor(x)
end

function dot(x1, y1, x2, y2)
    return x1 * x2 + y1 * y2
end

function rand1(x)
    return fract(sin(x) * 43758.5453123)
end

function rand2(x, y)
    return fract(sin(dot(x, y, 12.9898, 78.233)) * 43758.5453)
end

function distance(x1, y1, x2, y2)
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))
end

function length(x, y)
    return sqrt(x * x + y * y)
end

function step(edge, x)
    if x >= edge then
        return 1
    else
        return 0
    end
end

function remap(value, low1, high1, low2, high2)
    return low2 + (high2 - low2) * (value - low1) / (high1 - low1)
end

function round(x)
    return floor(x + 0.5)
end

function wave(x)
    return (cos((x + 0.5) * pi * 2) + 1) / 2
end

function isInRect(x, y, rectX, rectY, rectW, rectH)
    return x >= rectX and y >= rectY and x < rectX + rectW and y < rectY + rectH
end

function hsvToRgb(h,s,v)
    h = h % 360

    local c = v * s
    local x = c * (1 - math.abs((h / 60) % 2 - 1))
    local m = v - c

    local r = 0
    local g = 0
    local b = 0
    if h < 60 then
        r = c
        g = x
    elseif h < 120 then
        r = x
        g = c
    elseif h < 180 then
        g = c
        b = x
    elseif h < 240 then
        g = x
        b = c
    elseif h < 300 then
        r = x
        b = c
    else
        r = c
        b = x
    end

    return (r+m)*255, (g+m)*255, (b+m)*255
end

function over(r1,g1,b1,a1,r2,g2,b2,a2)
    a1 = a1 / 255
    a2 = a2 / 255
    local t = a1 * (1 - a2)
    local a = a2 + t
    if a == 0 then
        return 0, 0, 0, 0
    end
    local r = ((r2 / 255) * a2 + (r1 / 255) * t) / a
    local g = ((g2 / 255) * a2 + (g1 / 255) * t) / a
    local b = ((b2 / 255) * a2 + (b1 / 255) * t) / a
    return r * 255, g * 255, b * 255, a * 255
end

function extractRGBA(num)
    return band(rshift(num, 16), 0xff), band(rshift(num, 8), 0xff), band(num, 0xff), band(rshift(num, 24), 0xff)
end
)";
