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
            id = "color",
            type = "color",
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
    local progress = saturate(seconds)
    if progress > 0 and progress < 1 then
        progress = easing(progress)
    end

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 25
            local blue = 0
            local alpha = 255

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end

    for i = 0, count - 1 do
        local char = chars[i]
        local textX = textPos.x + char.x * progress
        local textY = textPos.y + char.y
        for ty = 0, char.h - 1 do
            for tx = 0, char.w - 1 do
                local x = floor(tx + textX)
                local y = floor(ty + textY)
                if x >= fromX and y >= fromY and x <= toX and y <= toY then
                    local pixel = getPixel(t, ts, tx, ty, i)
                    local value = smoothstep(-0.05, 0.05, pixel)
                    local existing = frame[y * width + x]
                    local red, green, blue, alpha = extractRGBA(existing)
                    red, green, blue, alpha = over(red, green, blue, alpha, color.r, color.g, color.b, color.a * value)
                    frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
                end
            end
        end
    end
end
