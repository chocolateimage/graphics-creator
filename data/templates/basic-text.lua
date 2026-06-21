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
            max = 1024,
            default = 128,
        },
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    local ti = createText(text, font)
    local _, _, _, _, w, h = getTextInfo(ti, fontSize)

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
                local value = smoothstep(-0.03, 0.03, getPixel(ti, fontSize, tx, ty))
                red = 255
                green = 255
                blue = 255
                alpha = value * 255
            end

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
