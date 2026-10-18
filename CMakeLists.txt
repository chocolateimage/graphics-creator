cmake_minimum_required(VERSION 3.26)

project(GraphicsCreator LANGUAGES CXX)

option(UPDATE_CHECK "Enable update check" ON)

if(UPDATE_CHECK)
    add_definitions(-DUPDATE_CHECK)
endif(UPDATE_CHECK)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_FIND_MODULE_DIR})

find_package(FFmpeg REQUIRED COMPONENTS
    AVCODEC
    AVFORMAT
    AVUTIL
    SWSCALE
)

find_package(HarfBuzz REQUIRED)
find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg Network)
find_package(KF6 REQUIRED IconThemes WidgetsAddons ColorScheme ConfigWidgets)
find_package(qtadvanceddocking-qt6 REQUIRED)

find_package(PkgConfig REQUIRED)

set(graphicscreator_SRCS
    src/main.cpp
    src/render.cpp
    src/variant.cpp
    src/scene.cpp
    src/brush.cpp
    src/priority_mutex.cpp
    src/log.cpp
    src/plugin.cpp
    src/presets.cpp
    src/project_templates.cpp
    src/gui/gui.cpp
    src/gui/image_viewer.cpp
    src/gui/fontcombobox.cpp
    src/gui/draggable_spinbox.cpp
    src/gui/video_file_button.cpp
    src/gui/brush_input.cpp
    src/gui/property_window.cpp
    src/gui/property_edit.cpp
    src/gui/effects_window.cpp
    src/gui/render_window.cpp
    src/gui/push_button.cpp
    src/gui/welcome_screen.cpp
    src/gui/flowlayout.cpp
    src/gui/welcome_popup.cpp
    src/gui/plugin_manager.cpp
    src/gui/color_picker.cpp
    src/gui/color_button.cpp
    src/gui/presets_window.cpp
    src/gui/ae_custom_ui.cpp
    src/gui/timeline/timeline.cpp
    src/gui/timeline/timeline_content.cpp
    src/gui/timeline/property_button.cpp
    src/gui/timeline/element_button.cpp
    src/gui/editors/editor.cpp
    src/gui/editors/text_element_editor.cpp
    src/gui/editors/path_element_editor.cpp
    src/animatable/animatable.cpp
    src/animatable/element/element.cpp
    src/animatable/element/rectangle_element.cpp
    src/animatable/element/ellipse_element.cpp
    src/animatable/element/text_element.cpp
    src/animatable/element/image_element.cpp
    src/animatable/element/group_element.cpp
    src/animatable/element/video_element.cpp
    src/animatable/element/path_element.cpp
    src/resources/graphics-creator.rc
)

add_subdirectory(src/animatable/effect)

add_subdirectory(src/ae)
if(WIN32)
    add_library(dvacore SHARED src/ae/libs/dvacore.cpp src/ae/libs/dvacore.def)
    add_library(VideoFrame SHARED src/ae/libs/videoframe.cpp src/ae/libs/videoframe.def)
    add_library(GPUFoundation SHARED src/ae/libs/gpufoundation.cpp src/ae/libs/gpufoundation.def)
    add_library(U SHARED src/ae/libs/u.cpp src/ae/libs/u.def)
    add_library(Memory SHARED src/ae/libs/memory.cpp)
    add_library(dvamediatypes SHARED src/ae/libs/dvamediatypes.cpp src/ae/libs/dvamediatypes.def)
    add_library(mediafoundation SHARED src/ae/libs/mediafoundation.cpp src/ae/libs/mediafoundation.def)
    add_library(PF SHARED src/ae/libs/pf.cpp src/ae/libs/pf.def)
    add_library(MSK SHARED src/ae/libs/msk.cpp)
    add_library(ColorSpaceConverter SHARED src/ae/libs/colorspaceconverter.cpp src/ae/libs/colorspaceconverter.def)
    add_library(OM SHARED src/ae/libs/om.cpp src/ae/libs/om.def)
    SET_TARGET_PROPERTIES(dvacore PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(VideoFrame PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(GPUFoundation PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(U PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(Memory PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(dvamediatypes PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(mediafoundation PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(PF PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(MSK PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(ColorSpaceConverter PROPERTIES PREFIX "")
    SET_TARGET_PROPERTIES(OM PROPERTIES PREFIX "")
endif(WIN32)

add_executable(graphics-creator ${graphicscreator_SRCS})
set_property(TARGET graphics-creator PROPERTY AUTOMOC ON)
set_property(TARGET graphics-creator PROPERTY CXX_STANDARD 17)

target_include_directories(graphics-creator PRIVATE ${FFMPEG_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} src)
target_link_libraries(
    graphics-creator
    Qt6::Core Qt6::Widgets Qt6::Svg Qt6::Network
    ads::qtadvanceddocking-qt6
    KF6::ColorScheme KF6::ConfigWidgets KF6::IconThemes KF6::WidgetsAddons
    FFmpeg::FFmpeg
    harfbuzz Freetype::Freetype Fontconfig::Fontconfig
)
target_compile_options(graphics-creator PRIVATE -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -mavx -mavx2)

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
