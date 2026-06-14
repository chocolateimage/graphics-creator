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
            id = "backgroundColor",
            type = "brush",
            default = {
                type = "linearGradient",
                color1 = {r = 5, g = 61, b = 139},
                color2 = {r = 17, g = 100, b = 176},
                angle = 45,
            }
        },
        {
            id = "textColor",
            type = "brush",
        },
        {
            id = "enterEasing",
            type = "easing",
            default = "easeInQuad",
        },
        {
            id = "expandEasing",
            type = "easing",
            default = "easeOutQuad",
        },
        {
            id = "closeEasing",
            type = "easing",
            default = "easeOutQuad",
        }
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    local ti = createText(text, font)
    local _, minY, _, _, w, h = getTextInfo(ti, fontSize)
    local textPosX = width / 2 - w / 2
    local textPosY = height / 2 + minY + fontSize / 3
    local enterProgress = enterEasing(saturate(seconds / .5))
    local expandProgress = expandEasing(saturate((seconds - 0.5) / .3))
    local middleSize = mix(fontSize * 1.5, height, expandProgress)
    if seconds > duration - 0.5 then
        local closeProgress = 1 - closeEasing(saturate(linearstep(duration - 0.3, duration, seconds)))
        middleSize = middleSize * closeProgress
    end
    local middleStart = height / 2 - middleSize / 2

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 0
            local alpha = 0

            local tx = x - textPosX
            local ty = y - textPosY
            if y >= middleStart and y < height / 2 + middleSize / 2 and x < enterProgress * width then
                red, green, blue, alpha = backgroundColor(x,y,width,height)
                if tx >= 0 and ty >= 0 and tx < w and ty < h then
                    local value = smoothstep(-0.03, 0.03, getPixel(ti, fontSize, tx, ty))
                    local r,g,b,a = textColor(tx,ty,w,h)
                    red, green, blue, alpha = over(red, green, blue, alpha, r,g,b,a*value)
                end
            end

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
