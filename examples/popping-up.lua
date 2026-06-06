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
            max = 500,
        },
        {
            id = "bottomLeft",
            type = "vector2dint",
        },
        {
            id = "borderSize",
            type = "int",
        },
        {
            id = "borderColor",
            type = "color",
        },
        {
            id = "backgroundColor",
            type = "color",
        },
        {
            id = "textColor",
            type = "color",
        },
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)
    local bottomAppearDuration = .5
    local bottomAppearProgress = easeOutExpo(saturate(seconds / bottomAppearDuration))
    local heightProgress = easeOutExpo(saturate((seconds - 0.2) * 2))
    local ti = createText(text, font)
    local minX, minY, maxX, maxY, tw, th = getTextInfo(ti, fontSize)
    local padding = 16
    local rectWidth = tw + padding + padding
    local rectWidth2 = bottomAppearProgress * rectWidth
    local rectHeight = (th + padding + padding) * heightProgress
    local textX = bottomLeft.x + padding
    local textY = bottomLeft.y - rectHeight + padding
    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 0
            local alpha = 0

            if x >= bottomLeft.x and x < rectWidth2 + bottomLeft.x and y >= bottomLeft.y and y < bottomLeft.y + borderSize then
                red = borderColor.r
                green = borderColor.g
                blue = borderColor.b
                alpha = borderColor.a
            end

            if x >= bottomLeft.x and x < rectWidth2 + bottomLeft.x and y >= bottomLeft.y - rectHeight and y < bottomLeft.y then
                red = backgroundColor.r
                green = backgroundColor.g
                blue = backgroundColor.b
                alpha = backgroundColor.a

                local pixel = getPixel(ti, fontSize, x - textX, y - textY)
                local value = smoothstep(-0.05, 0.05, pixel)
                red = mix(red, textColor.r, value)
                green = mix(green, textColor.g, value)
                blue = mix(blue, textColor.b, value)
                alpha = mix(alpha, textColor.a, value)
            end

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
