-- SPDX-License-Identifier: MIT

function options()
    return {}
end

function draw(_frame)
    local frame = ffi.cast("uint32_t*", _frame)

    for y = fromY, toY do
        for x = fromX, toX do
            local red = 0
            local green = 0
            local blue = 0
            local alpha = 255

            -- Put your draw code here!

            frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
        end
    end
end
