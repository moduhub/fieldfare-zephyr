cmake_minimum_required(VERSION 3.22)

set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(fieldfare-zephyr)

add_subdirectory(src)

include_directories(src)