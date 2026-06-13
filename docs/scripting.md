# Scripting

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

- `createText(text, font)`: Creates a TextClass instance. The `font` argument must be from a `font` data type from an option.
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
- `over(r1,g1,b1,a1,r2,g2,b2,a2)`: Adds together two transparent values. First four values (r1, g1, b1, a1) are "behind", and the last four values (r2, g2, b2, a2) are "over".\
  You would use it like this:
  ```lua
  -- "red, green, blue, alpha" comes from the frame.
  -- "255, 0, 0, 128" means half transparent red.
  red, green, blue, alpha = over(red, green, blue, alpha, 255, 0, 0, 128)
  ```
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

- `options()`: In this function, you should return the available options that you can change in the UI. More info is available info the [Options](#Options) section.

  Example with a modifiable `double` option:

  ```lua
  function options()
      return {
          {
              id = "exampleOption",
              type = "double",
          }
      }
  end
  ```

## Options

These option data types are available. They can be set in the `type` property. In the examples, `<id>` is the id property.

- `string`
- `int`\
  Available properties:
  - `min = 0` (int): Minimum value of the property.
  - `max = 100` (int): Maximum value of the property.
  - `slider = false` (boolean): If a slider should be shown instead of an input box. Requires `min` and `max` to be useful.
- `double`\
  Available properties:
  - `min = 0` (int): Minimum value of the property.
  - `max = 100` (int): Maximum value of the property.
- `bool`
- `vector2dint`: Two ints in one variable. The two ints can be accessed with `<id>.x` and `<id>.y`
- `color`: RGBA color. Can be accessed with `<id>.r`, `<id>.g`, `<id>.b`, and for alpha `<id>.a`. All values are in 0-255.\
  <img width="517" height="396" alt="image" src="https://github.com/user-attachments/assets/83386caa-1a99-4344-8cea-c75822925df9" />

- `font`: Font to be passed in `createText()`\
  <img width="401" height="411" alt="image" src="https://github.com/user-attachments/assets/35b37fa5-4571-4dbe-99d0-148ad5fd39e8" />

- `easing`: Selectable list of easings. The value in Lua will be the specific easing function and can be used like this: `local value = <id>(0.5)`\
  <img width="168" height="274" alt="image" src="https://github.com/user-attachments/assets/384dea3b-251f-4f7c-946a-9e5ffb2150b6" />
