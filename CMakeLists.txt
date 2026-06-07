cmake_minimum_required(VERSION 3.26)

project(GraphicsCreator LANGUAGES CXX)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_FIND_MODULE_DIR})

find_package(FFmpeg REQUIRED COMPONENTS
    AVFORMAT
    SWSCALE
    AVUTIL
    AVCODEC
    AVFILTER
    AVDEVICE
    SWRESAMPLE
)

find_package(HarfBuzz REQUIRED)
find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(KF6 REQUIRED TextEditor IconThemes WidgetsAddons)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LuaJIT REQUIRED IMPORTED_TARGET luajit)

set(graphicscreator_SRCS
    src/gui.cpp
    src/render.cpp
    src/video.cpp
    src/image_viewer.cpp
    src/lua_state.cpp
    src/variant.cpp
    src/fontcombobox.cpp
    src/draggable_spinbox.cpp
)


add_executable(graphics-creator WIN32 ${graphicscreator_SRCS})
set_property(TARGET graphics-creator PROPERTY AUTOMOC ON)

target_include_directories(graphics-creator PRIVATE ${FFMPEG_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${LUAJIT_INCLUDE_DIRS})
target_link_libraries(graphics-creator Qt6::Core Qt6::Widgets FFmpeg::FFmpeg harfbuzz Freetype::Freetype KF6::TextEditor KF6::IconThemes KF6::WidgetsAddons PkgConfig::LuaJIT Fontconfig::Fontconfig)
target_compile_options(graphics-creator PRIVATE -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter)

add_custom_command(TARGET graphics-creator POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                       ${CMAKE_CURRENT_SOURCE_DIR}/data
                       $<TARGET_FILE_DIR:graphics-creator>/data
                   )
