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
find_package(qtadvanceddocking-qt6 REQUIRED)

find_package(PkgConfig REQUIRED)
#pkg_check_modules(LuaJIT REQUIRED IMPORTED_TARGET luajit)

set(graphicscreator_SRCS
    src/render.cpp
    src/video.cpp
    #src/lua_state.cpp
    src/variant.cpp
    src/scene.cpp
    src/brush.cpp
    src/gui/gui.cpp
    src/gui/image_viewer.cpp
    src/gui/fontcombobox.cpp
    src/gui/draggable_spinbox.cpp
    src/gui/video_file_button.cpp
    src/gui/brush_input.cpp
    src/gui/property_window.cpp
    src/gui/property_edit.cpp
    src/gui/effects_window.cpp
    src/gui/text_element_editor.cpp
    src/gui/render_window.cpp
    src/gui/push_button.cpp
    src/gui/welcome_screen.cpp
    src/gui/flowlayout.cpp
    src/gui/welcome_popup.cpp
    src/gui/timeline/timeline.cpp
    src/gui/timeline/timeline_content.cpp
    src/gui/timeline/property_button.cpp
    src/gui/timeline/element_button.cpp
    src/animatable/animatable.cpp
    src/animatable/element/element.cpp
    src/animatable/element/rectangle_element.cpp
    src/animatable/element/ellipse_element.cpp
    src/animatable/element/text_element.cpp
    src/animatable/element/image_element.cpp
    src/resources/graphics-creator.rc
)

add_subdirectory(src/animatable/effect)


add_executable(graphics-creator WIN32 ${graphicscreator_SRCS})
set_property(TARGET graphics-creator PROPERTY AUTOMOC ON)

target_include_directories(graphics-creator PRIVATE ${FFMPEG_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} src)
target_link_libraries(graphics-creator Qt6::Core Qt6::Widgets FFmpeg::FFmpeg harfbuzz Freetype::Freetype KF6::TextEditor KF6::IconThemes KF6::WidgetsAddons Fontconfig::Fontconfig ads::qtadvanceddocking-qt6)
target_compile_options(graphics-creator PRIVATE -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter)

add_custom_command(TARGET graphics-creator POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                       ${CMAKE_CURRENT_SOURCE_DIR}/data
                       $<TARGET_FILE_DIR:graphics-creator>/data
                   )

install(TARGETS graphics-creator)
install(DIRECTORY data DESTINATION share/graphics-creator)
install(FILES me.chocolateimage.graphics-creator.desktop DESTINATION share/applications)
install(FILES me.chocolateimage.graphics-creator.metainfo.xml DESTINATION share/metainfo)
install(FILES me.chocolateimage.graphics-creator.mime.xml DESTINATION share/mime/packages)
install(FILES src/icons/icon.svg DESTINATION share/icons/hicolor/scalable/apps RENAME me.chocolateimage.graphics-creator.svg)
install(FILES src/icons/icon-16.png DESTINATION share/icons/hicolor/16x16/apps RENAME me.chocolateimage.graphics-creator.png)
install(FILES src/icons/icon-32.png DESTINATION share/icons/hicolor/32x32/apps RENAME me.chocolateimage.graphics-creator.png)
install(FILES src/icons/icon-48.png DESTINATION share/icons/hicolor/48x48/apps RENAME me.chocolateimage.graphics-creator.png)
install(FILES src/icons/icon-64.png DESTINATION share/icons/hicolor/64x64/apps RENAME me.chocolateimage.graphics-creator.png)
install(FILES src/icons/icon-128.png DESTINATION share/icons/hicolor/128x128/apps RENAME me.chocolateimage.graphics-creator.png)
install(FILES src/icons/icon-256.png DESTINATION share/icons/hicolor/256x256/apps RENAME me.chocolateimage.graphics-creator.png)
install(FILES src/icons/icon-512.png DESTINATION share/icons/hicolor/512x512/apps RENAME me.chocolateimage.graphics-creator.png)
