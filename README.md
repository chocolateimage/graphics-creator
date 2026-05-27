# Graphics Creator

> [!WARNING]
> this tool is **work in progress**. IT'S NOT READY!!!

*Name is undecided.*

Create titles and other visual effects with Lua.

Supports Linux and Windows.

## Planned

- Defaults for config
- Config presets
- Draggable spinboxes
- Templates for scripts (you can pick basic text/rainbow text/moving circles/etc at the start)
- Rendering (I have a working source code file for rendering, just need to add that and make a nice UI for it)

## Scripting

TODO: put this somewhere else instead of the README (in the program itself?)

Available variables:

- `frameIndex`: Frame index starting from 0
- `frameRate`: FPS
- `width`: Width of the video in pixels
- `height`: Height of the video in pixels
- `seconds`: Position in video in seconds
- `duration`: Duration of the video in seconds
- `fromX`: Start of region to draw (x coord)
- `fromY`: Start of region to draw (y coord)
- `toX`: End of region to draw (x coord)
- `toY`: End of region to draw (y coord)

Available functions:

- `createText(text, unused)`: Creates a TextClass instance. TODO: `unused` will later be the font options.
- `getTextInfo(textClassInstance, fontSize)`: Returns `minX, minY, maxX, maxY, textWidth, textHeight`.
- `getPixel(textClassInstance, fontSize, x, y)`: Returns a value from -0.5 to 0.5 of the SDF of the text with the font size. Usually used like `smoothstep(-0.05, 0.05, getPixel(text, 64, x - 100, y - 100)) * 255` to get a value from 0 to 255.
- `mix(from, to, x)`/`lerp(from, to, x)`: Both do the same. Converts a value from x (0 to 1 range) to `from` - `to` range.
- `clamp(x, from, to)`. Clamps a value from `from` to `to`.
- `saturate(x)`. Clamps a value from 0 to 1
- `linearstep(from, to, x)`. Reverse of lerp. Transforms `x` which is from `from` to `to` to `0` to `1` (clamped).
- `smoothstep(from, to, x)`: Same as linearstep, but [smooth](https://en.wikipedia.org/wiki/Smoothstep).
- `fract(x)`: Returns the decimal value of x. Same as `x % 1`.
- `rand1(x)`: Basic 1D random. Put in a value in x and it returns a deterministic random value from 0 to 1.
- `rand2(x, y)`: Basic 2D random. Put in a value in x and y and it returns a deterministic random value from 0 to 1.
- `step(edge, x)`: Returns 1 if x >= edge, else 0.
- `remap(value, low1, high1, low2, high2)`: Remaps `value` from `low1` - `high1` to `low2` - `high2`.
- `round(x)`: Rounds down and up to the nearest integer.
- `wave(x)`: Sine wave but instead of -1 to 1, it goes from 0 to 1. When x is 0, it's the start of the wave. When x is 0.5, it's the peak of the wave, and when x is 1 it's back to the start of the wave.
- `isInRect(x, y, rectX, rectY, rectW, rectH)`: Returns `true`/`false` when the coords are in the rectangle or not.
- `hsvToRgb(hue, saturation, value)`: Hue is from 0 to 360. Saturation and value are from 0 to 1.
- all `math.` functions are in global scope (`math.sin()` can be used just with `sin()`)
- easing functions from [easings.net](https://easings.net). call it like `easeOutSine(x)`

Functions you should define:

- `draw(_frame)`: `_frame` is the raw frame userdata.

    Basic example:

    ```lua
    function draw(_frame)
        local frame = ffi.cast("uint32_t*", _frame)
        
        for y = fromY, toY do
            for x = fromX, toX do
                local red = 0
                local green = 0
                local blue = 0
                local alpha = 255

                frame[y * width + x] = bor(lshift(alpha, 24), lshift(red, 16), lshift(green, 8), blue)
            end
        end
    end
    ```

- `options()`: WIP
