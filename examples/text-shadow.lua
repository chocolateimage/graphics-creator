-- SPDX-License-Identifier: MIT

function options()
    return {
        {
            id = "font",
            type = "font",
        },
        {
            id = "text",
            type = "string",
            default = "Hello world",
        },
        {
            id = "textPos",
            type = "vector2dint",
            default = { x = 200, y = 200 },
        },
        {
            id = "fontSize",
            type = "int",
            min = 0,
            max = 1000,
            slider = true,
            default = 128,
        },
        {
            id = "innerColor",
            type = "color",
            default = { r = 255, g = 255, b = 255, a = 255 },
        },
        {
            id = "borderColor",
            type = "color",
            default = { r = 0, g = 0, b = 0, a = 255 },
        },
        {
            id = "shadowColor",
            type = "color",
            default = { r = 0, g = 87, b = 194, a = 255 },
        },
        {
            id = "shadowWithBorder",
            type = "bool",
            default = true,
        },
        {
            id = "shadowSize",
            type = "int",
            min = 0,
            max = 100,
            default = 10,
        }
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    local ti = createText(text, font)
    local _, _, _, _, w, h = getTextInfo(ti, fontSize)

    local shadowFrom = -0.05
    local shadowTo = 0.05
    if shadowWithBorder then
        shadowFrom = -0.5
        shadowTo = -0.4
    end

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 0
            local alpha = 0

            local tx = x - textPos.x
            local ty = y - textPos.y
            if tx >= 0 and ty >= 0 and tx <= w + shadowSize and ty <= h + shadowSize then
                local value = smoothstep(-0.05, 0.05, getPixel(ti, fontSize, tx, ty))
                local border = smoothstep(-0.5, -0.4, getPixel(ti, fontSize, tx, ty))
                for i = 1, shadowSize do
                    local shadow = smoothstep(shadowFrom, shadowTo, getPixel(ti, fontSize, tx - i, ty - i))
                    red = mix(red, shadowColor.r, shadow)
                    green = mix(green, shadowColor.g, shadow)
                    blue = mix(blue, shadowColor.b, shadow)
                    alpha = mix(alpha, shadowColor.a, shadow)
                end
                red = mix(red, borderColor.r, border)
                green = mix(green, borderColor.g, border)
                blue = mix(blue, borderColor.b, border)
                alpha = mix(alpha, borderColor.a, border)
                red = mix(red, innerColor.r, value)
                green = mix(green, innerColor.g, value)
                blue = mix(blue, innerColor.b, value)
                alpha = mix(alpha, innerColor.a, value)
            end

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
