-- SPDX-License-Identifier: MIT

function options()
    return {
        {
            id = "aBrush",
            type = "brush",
        }
    }
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    for y = fromY, toY do
        for x = fromX, toX do
            local red, green, blue, alpha = aBrush(x, y, width, height)

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
