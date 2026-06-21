-- SPDX-License-Identifier: MIT

function options()
    return {
        {
            type = "color",
            id = "color",
            default = "#3b82f6",
        }
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    local sizeP = mix(0.5, 1, easeOutCubic(saturate(seconds / .2)))
    local rem = (1 - easeOutCubic(saturate(seconds / .5))) * 150
    local show = easeOutCubic(saturate(seconds / .2))
    local size = 500 * sizeP + 1
    if size - rem < 0 then
        rem = size
    end

    for y = fromY, toY do
        for x = fromX, toX do
            local red = color.r
            local green = color.g
            local blue = color.b
            local alpha = 255

            local dist = smoothstep(0.5, 0.497, saturate(distance(x, y, width / 2, height / 2) / (size)))
            local dist2 = smoothstep(0.5, 0.497, saturate(distance(x, y, width / 2, height / 2) / ((size) - rem)))

            alpha = (dist - dist2) * 255 * show

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
