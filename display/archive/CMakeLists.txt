cmake_minimum_required(VERSION 3.10)

project(display VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets REQUIRED)

qt_standard_project_setup()


add_library(display STATIC src/display.cpp inc/display.hpp)
target_include_directories(display PUBLIC inc)
target_include_directories(display PUBLIC /media/matt/HDD1/buildroot-2025.02.10/output/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/QtWidgets)

target_link_libraries(display PRIVATE Qt6::Core)
target_link_libraries(display PRIVATE Qt6::Widgets)