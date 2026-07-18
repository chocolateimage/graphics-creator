#!/usr/bin/env python3
import argparse
import os

parser = argparse.ArgumentParser()
parser.add_argument("name", help="Name in PascalCase")

args = parser.parse_args()

pascal_case_name = args.name
snake_case_name = ""
space_name = ""

for i, v in enumerate(pascal_case_name):
    if v.upper() == v and i > 0:
        snake_case_name += "_"
        space_name += " "
    snake_case_name += v.lower()
    space_name += v

pascal_case_name += "Effect"
snake_case_name += "_effect"

header_contents = f"""#pragma once
#include "effect.hpp"

class {pascal_case_name}Render : public EffectRender {{
  public:
    ~{pascal_case_name}Render() {{}}
    bool render(const uint32_t *source, const Rect &sourceRect,
                uint32_t *target) override;
}};

class {pascal_case_name} : public Effect {{
  public:
    {pascal_case_name}();
    ~{pascal_case_name}() {{}};
    QString effectName() override {{ return "{space_name}"; }};
    AnimatableRender *createClass() override {{
        return new {pascal_case_name}Render();
    }};
}};
"""

source_contents = f"""#include "{snake_case_name}.hpp"
#include "math.hpp"

{pascal_case_name}::{pascal_case_name}() {{}}

bool {pascal_case_name}Render::render(const uint32_t *source, const Rect &sourceRect,
                               uint32_t *target) {{
    Rect rect = renderBox;
    for (int y = 0; y < sourceRect.h; y++) {{
        for (int x = 0; x < sourceRect.w; x++) {{
            target[pixelIndex(x, y, rect.w)] =
                source[pixelIndex(x, y, sourceRect.w)];
        }}
    }}
    return true;
}}
"""

header_path = f"src/animatable/effect/{snake_case_name}.hpp"
source_path = f"src/animatable/effect/{snake_case_name}.cpp"
cmake_lists_path = "src/animatable/effect/CMakeLists.txt"

if os.path.exists(header_path) or os.path.exists(source_path):
    print("Already exists")
    exit(1)

if not os.path.exists(cmake_lists_path):
    print("Wrong working directory")
    exit(1)


with open(header_path, "w+") as f:
    f.write(header_contents)

with open(source_path, "w+") as f:
    f.write(source_contents)


with open(cmake_lists_path) as f:
    cmake = f.read()
cmake = cmake.replace("PARENT_SCOPE", f"{source_path}\n  PARENT_SCOPE")
with open(cmake_lists_path, "w+") as f:
    f.write(cmake)
