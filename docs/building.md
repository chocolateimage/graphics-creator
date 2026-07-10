# Building

1. Follow [Linux](#Linux) or [Windows](#Windows) preparation instructions.
2. Go to directory of this project: `cd /home/.../graphics-creator`
3. Create build dir: `mkdir build`
4. `cd build`
5. Prepare (at the end you can choose Debug/Release): `cmake .. -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Release`
6. Build: `ninja`
7. Run: `./graphics-creator`

## Linux

Install these packages:

```bash
# Arch
pacman -S --needed cmake ninja qt6-base qt-advanced-docking-system ffmpeg luajit kiconthemes kwidgetsaddons ktexteditor extra-cmake-modules breeze breeze-icons
```

## Windows

1. Install [MSYS2](https://www.msys2.org/)
1. Open the UCRT64 terminal (NOT one of the other ones)
1. Install `pactoys`:
   ```bash
   pacman -S pactoys
   ```
1. Install these packages:
   ```bash
   pacboy -S --needed gcc cmake ninja qt6-base qt-advanced-docking-system ffmpeg luajit kiconthemes kwidgetsaddons ktexteditor extra-cmake-modules breeze breeze-icons
   ```
