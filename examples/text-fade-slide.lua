-- SPDX-License-Identifier: MIT

function options()
    return {
        {
            id = "font",
            type = "font",
        },
        {
            id = "text",
            type = "string"
        },
        {
            id = "fontSize",
            type = "int",
            slider = true,
            min = 0,
            max = 1000,
        },
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    local ti = createText(text, font)
    local _, _, _, _, w, h = getTextInfo(ti, fontSize)
    local stripeWidth = 64
    local fadeWith = 16
    local stripeDuration = 2
    local stripeX = easeOutCubic(saturate(seconds / stripeDuration)) * (w + stripeWidth) - fadeWith
    local textPosX = width / 2 - w / 2
    local textPosY = height / 2 - h / 2

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 0
            local alpha = 0

            local tx = x - textPosX
            local ty = y - textPosY
            if tx >= 0 and ty >= 0 and tx < w and ty < h then
                local r, g, b = hsvToRgb(ty / h * 360, 1, 1)
                local value = smoothstep(-0.05, 0.05, getPixel(ti, fontSize, tx, ty))
                if tx > stripeX then
                    if tx < stripeX + stripeWidth then
                        local value2 = 1 - saturate((tx - stripeX) / fadeWith)
                        red = r
                        green = g
                        blue = b
                        alpha = value2 * value * 255
                    end
                else
                    local value2 = saturate((stripeX - tx) / stripeWidth)
                    red = mix(r, 255, value2)
                    green = mix(g, 255, value2)
                    blue = mix(b, 255, value2)
                    alpha = value * 255
                end
            end

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
