cmake_minimum_required(VERSION 3.26)

project(PluginTest LANGUAGES CXX)

add_library(sample-plugin SHARED plugin.cpp)

target_link_options(sample-plugin PRIVATE -static-libgcc -static-libstdc++)
