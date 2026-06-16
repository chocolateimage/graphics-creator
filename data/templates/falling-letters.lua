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
            default = { x = 50, y = 1000, },
        },
        {
            id = "color",
            type = "brush",
            default = { color1 = { r = 255, g = 255, b = 255 } }
        },
        {
            id = "easing",
            type = "easing",
            default = "easeOutBounce",
        },
        {
            id = "speed",
            type = "double",
            default = 1,
        },
        {
            id = "letterSpeed",
            type = "double",
            default = 1,
        }
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
            local green = 0
            local blue = 0
            local alpha = 0

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end

    for i = 0, count - 1 do
        local char = chars[i]
        local progress = saturate((seconds + ((count - i) / count - 1) / letterSpeed) * speed)
        local showProgress = linearstep(0, 0.1, progress)
        if progress > 0 and progress < 1 then
            progress = easing(progress)
        end
        if progress > 0 then
            local textX = textPos.x + char.x
            local textY = (textPos.y - 500 * (1 - progress)) + char.y
            for ty = 0, char.h - 1 do
                for tx = 0, char.w - 1 do
                    local x = floor(tx + textX)
                    local y = floor(ty + textY)
                    if x >= fromX and y >= fromY and x <= toX and y <= toY then
                        local pixel = getPixel(t, ts, tx, ty, i)
                        local value = smoothstep(-0.05, 0.05, pixel)
                        local existing = frame[y * width + x]
                        local red, green, blue, alpha = extractRGBA(existing)
                        local r, g, b, a = color(tx, ty, char.w, char.h)
                        red, green, blue, alpha = over(red, green, blue, alpha, r, g, b, a * value * showProgress)
                        frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
                    end
                end
            end
        end
    end
end
