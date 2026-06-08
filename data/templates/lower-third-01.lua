-- SPDX-License-Identifier: MIT

function options()
    return {
        {
            id = "font",
            type = "font",
        },
        {
            id = "subTitleFont",
            type = "font",
        },
        {
            id = "title",
            type = "string",
            default = "Title",
        },
        {
            id = "subtitle",
            type = "string",
            default = "Subtitle",
        },
        {
            id = "fontSizeTitle",
            type = "int",
            slider = true,
            min = 0,
            max = 1000,
            default = 128,
        },
        {
            id = "fontSizeSubTitle",
            type = "int",
            slider = true,
            min = 0,
            max = 1000,
            default = 90,
        },
        {
            id = "textPos",
            type = "vector2dint",
            label = "Text position",
            default = { x = 64, y = 900, },
        },
        {
            id = "titleColor",
            type = "color",
            default = { r = 255, g = 255, b = 255, a = 255 },
        },
        {
            id = "subTitleColor",
            type = "color",
            default = { r = 255, g = 255, b = 255, a = 255 },
        },
        {
            id = "lineColor",
            type = "color",
            default = { r = 255, g = 255, b = 255, a = 255 },
        },
        {
            id = "lineHeight",
            type = "int",
            min = 0,
            max = 1000,
            default = 4,
        },
        {
            id = "topSpacing",
            type = "int",
            min = -1000,
            max = 1000,
            default = 32,
        },
        {
            id = "topHorizontalSpacing",
            type = "int",
            min = -1000,
            max = 1000,
            default = 24,
        },
        {
            id = "bottomSpacing",
            type = "int",
            min = -1000,
            max = 1000,
        },
        {
            id = "bottomHorizontalSpacing",
            type = "int",
            min = -1000,
            max = 1000,
            default = 16,
        },
    }
end

function draw(_frame)
    local t = createText(title, font)
    local t2 = createText(subtitle, subTitleFont)
    local minX, minY, maxX, maxY, tw, th = getTextInfo(t, fontSizeTitle)
    local minX2, minY2, maxX2, maxY2, tw2, th2 = getTextInfo(t2, fontSizeSubTitle)
    local frame = ffi.cast("uint32_t*", _frame)

    local lineProgress = easeOutExpo(saturate(seconds / .5))
    local textMoveProgress = 1 - easeOutExpo(saturate((seconds - 0.2) / .3))
    local lineWidth = (max(tw + topHorizontalSpacing, tw2 + bottomHorizontalSpacing) + 32) * lineProgress

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 0
            local alpha = 0

            local textX = x - textPos.x - minX - topHorizontalSpacing
            local textY = y - textPos.y - minY + topSpacing - (textMoveProgress * (th + topSpacing))

            if textX >= 0 and textY >= 0 and textX < tw and textY < th and y < textPos.y then
                local pixel = getPixel(t, fontSizeTitle, textX, textY)
                local innerValue = smoothstep(-0.05, 0.05, pixel)

                red = titleColor.r
                green = titleColor.g
                blue = titleColor.b
                alpha = mix(alpha, subTitleColor.a, innerValue)
            end

            local textX = x - textPos.x - minX2 - bottomHorizontalSpacing
            local textY = y - textPos.y - minY2 - fontSizeSubTitle - bottomSpacing +
                (textMoveProgress * ((fontSizeSubTitle * 1.3) + bottomSpacing)) - lineHeight
            if textX >= 0 and textY >= 0 and textX < tw2 and textY < th2 and y >= textPos.y + lineHeight then
                local pixel = getPixel(t2, fontSizeSubTitle, textX, textY)
                local innerValue = smoothstep(-0.05, 0.05, pixel)

                red = subTitleColor.r
                green = subTitleColor.g
                blue = subTitleColor.b
                alpha = mix(alpha, subTitleColor.a, innerValue)
            end

            if x >= textPos.x and y >= textPos.y and x <= textPos.x + lineWidth and y <= textPos.y + lineHeight then
                red = lineColor.r
                green = lineColor.g
                blue = lineColor.b
                alpha = lineColor.a
            end

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
