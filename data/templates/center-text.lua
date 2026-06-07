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
            max = 1024,
            default = 128,
        },
        {
            id = "stretched",
            type = "bool",
        },
        {
            id = "backgroundColor",
            type = "color",
            default = { r = 0, g = 0, b = 255, a = 255 },
        },
        {
            id = "textColor",
            type = "color",
            default = { r = 255, g = 255, b = 255, a = 255 },
        }
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    local ti = createText(text, font)
    local _, _, _, _, w, h = getTextInfo(ti, fontSize)

    local progress = easeOutElastic(saturate(seconds / 1))
    local origW = width
    local origH = height
    if not stretched then
        origW = w + 64
        origH = h + 64
    end
    local endX = progress * origW
    local endY = progress * origH
    local moveX = width / 2 - endX / 2
    local moveY = height / 2 - endY / 2
    local stretchX = progress
    local stretchY = progress
    local textMoveX = 0
    local textMoveY = 0
    if stretched then
        stretchX = endX / w
        stretchY = endY / h
    else
        textMoveX = endX / 2 - w / 2 * stretchX
        textMoveY = endY / 2 - h / 2 * stretchY
    end

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 0
            local alpha = 0

            local tx = (x - moveX - textMoveX) / stretchX
            local ty = (y - moveY - textMoveY) / stretchY
            if x > moveX and y > moveY and x < (endX + moveX) and y < (endY + moveY) then
                local value = smoothstep(-0.03, 0.03, getPixel(ti, fontSize, tx, ty))
                red = mix(backgroundColor.r, textColor.r, value)
                green = mix(backgroundColor.g, textColor.g, value)
                blue = mix(backgroundColor.b, textColor.b, value)
                alpha = mix(backgroundColor.a, textColor.a, value)
            end


            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
