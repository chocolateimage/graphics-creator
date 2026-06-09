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
            id = "fontSize",
            type = "int",
            slider = true,
            min = 0,
            max = 1000,
            default = 128,
        },
        {
            id = "textPos",
            type = "vector2dint",
            label = "Text position",
            default = { x = 400, y = 400, },
        },
        {
            id = "easing",
            type = "easing",
        },
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    local t = createText(text, font, true)
    local ts = fontSize

    local chars = getAllCharsInfo(t, ts)

    local count = #t

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 0
            local alpha = 255

            for i = 0, count - 1 do
                local progress = saturate(seconds + (count - i) / count - 1)
                if progress > 0 and progress < 1 then
                    progress = easing(progress)
                end
                local char = chars[i]
                local textX = x - textPos.x - char.x
                local textY = y - textPos.y - char.y - progress * 500
                if textX >= 0 and textY >= 0 and textX < char.w and textY < char.h then
                    local pixel = getPixel(t, ts, textX, textY, i)
                    local innerValue = smoothstep(-0.05, 0.05, pixel)

                    red = mix(red, 255, innerValue)
                    green = mix(green, 255, innerValue)
                    blue = mix(blue, 255, innerValue)
                end
            end

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
