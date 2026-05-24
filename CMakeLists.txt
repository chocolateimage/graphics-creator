cmake_minimum_required(VERSION 3.16)

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
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(KF6 REQUIRED TextEditor IconThemes)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LuaJIT REQUIRED IMPORTED_TARGET luajit)

set(graphicscreator_SRCS
    src/gui.cpp
    src/render.cpp
    src/video.cpp
    src/image_viewer.cpp
    src/lua_state.cpp
    src/variant.cpp
)


add_executable(graphicscreator WIN32 ${graphicscreator_SRCS})
set_property(TARGET graphicscreator PROPERTY AUTOMOC ON)

target_include_directories(graphicscreator PRIVATE ${FFMPEG_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${LUAJIT_INCLUDE_DIRS})
target_link_libraries(graphicscreator Qt6::Core Qt6::Widgets ${FFMPEG_LIBRARIES} harfbuzz Freetype::Freetype ${LUAJIT_LIBRARIES} KF6::TextEditor KF6::IconThemes PkgConfig::LuaJIT)
target_compile_options(graphicscreator PRIVATE -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter)
