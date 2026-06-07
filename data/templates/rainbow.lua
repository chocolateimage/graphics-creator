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
        },
        {
            id = "fontSize",
            type = "int",
            slider = true,
            min = 0,
            max = 1000,
        },
        {
            id = "textPos",
            type = "vector2dint",
            label = "Text position",
        },
        {
            id = "borderColor",
            type = "color",
        },
    }
end

function draw(_frame)
    local full = text
    local t = createText(string.sub(full, 0, seconds * #full), font)
    local ts = fontSize
    local minX, minY, maxX, maxY, tw, th = getTextInfo(t, ts)
    local frame = ffi.cast("uint32_t*", _frame)

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 255
            local green = 255
            local blue = 255
            local alpha = 0
            local textX = x - textPos.x - minX
            local textY = y - textPos.y - minY
            local pixel = getPixel(t, ts, textX, textY)
            local innerValue = smoothstep(-0.05, 0.05, pixel)

            local hr, hg, hb = hsvToRgb(((textX / tw) - seconds) * 360, 1, 1)
            red = mix(red, hr, innerValue)
            green = mix(green, hg, innerValue)
            blue = mix(blue, hb, innerValue)

            local glowValue = 1 - innerValue
            red = mix(red, borderColor.r, glowValue)
            green = mix(green, borderColor.g, glowValue)
            blue = mix(blue, borderColor.b, glowValue)
            glowValue = (smoothstep(-0.3, -0.2, pixel) - innerValue)
            alpha = saturate(innerValue + glowValue) * 255

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
