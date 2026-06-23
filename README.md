# Graphics Creator

> [!WARNING]
> this tool is **work in progress**. IT'S NOT READY!!!

_Name is undecided._

Create titles and other visual effects with Lua.

Supports Linux and Windows.

## Planned

- Config presets
- Save templates (with recents)

This version of the tool is currently only a prototype, so right now only people with experience in Lua and interested in scripting can make their own templates. I want people to be able to make templates themselves without needing to script ([suggestion](https://discuss.kde.org/t/i-made-a-tool-for-kdenlive-for-titles-and-graphics/47773/13)). Therefore I have several ideas:

- Major overhaul. Instead of a single script view, you can add elements in the "scene" (or how it will be called).
- Still keep Lua scripting, but make it into a separate element type
- Basic element types like: Box, Circle, Text
- For these basic element types like box, I want them to be able to use a brush (brushes being the existing color/linear-gradient/radial-gradient)
- Being able to group elements together
- You can add effects to elements/groups
  - Effects can either change things visually or transform it
  - Effects would also be scriptable
- Animate elements and properties with a timeline
- Instead of fixed layout it is dockable

## Scripting

If you want to make your own Lua template, then look at [the scripting docs](docs/scripting.md)

## Contributing

It would be really helpful for me if, in case you have an idea, or found a bug, that you [create an issue](https://github.com/chocolateimage/graphics-creator/issues/new)!

In case you are a developer and want to add a new feature to the program, or add a new template, then you are welcome to create a PR! (just no AI slop please)

- License of the templates: [MIT](data/templates/LICENSE)
- License of the rest of the program: [GPLv3](LICENSE)
