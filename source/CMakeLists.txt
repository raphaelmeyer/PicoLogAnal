cmake_minimum_required(VERSION 3.18)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(PicoLogAnal)

pico_sdk_init()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(picologanal)

target_sources(picologanal PRIVATE
  main.cc
  st7735.cc
)

target_link_libraries(picologanal PRIVATE
  pico_stdlib
  hardware_spi
)

target_compile_options(picologanal PRIVATE -Wall -Wextra -Werror -Wno-ignored-qualifiers)

pico_add_extra_outputs(picologanal)

