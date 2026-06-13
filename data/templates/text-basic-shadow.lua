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
            id = "textColor",
            type = "brush",
        }
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
                local value = smoothstep(-0.5, 0, getPixel(ti, fontSize, tx, ty))
                red = 0
                green = 0
                blue = 0
                alpha = value * 255
                local value = smoothstep(-0.05, 0.05, getPixel(ti, fontSize, tx, ty))
                local r,g,b,a = textColor(tx,ty,w,h)
                red, green, blue, alpha = over(red, green, blue, alpha, r,g,b,a*value)
            end

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
