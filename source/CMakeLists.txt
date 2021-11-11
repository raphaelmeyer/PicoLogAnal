cmake_minimum_required(VERSION 3.18)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(PicoLogAnal)

pico_sdk_init()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(picologanal)

pico_generate_pio_header(picologanal ${CMAKE_CURRENT_SOURCE_DIR}/sampling.pio)

target_sources(picologanal PRIVATE
  main.cc
  display.cc
  pico_log_anal.cc
  st7735.cc

  test_signal.cc
)

target_link_libraries(picologanal PRIVATE
  pico_multicore
  pico_stdlib
  hardware_adc
  hardware_dma
  hardware_pio
  hardware_spi

  hardware_pwm
)

target_compile_options(picologanal PRIVATE -Wall -Wextra -Werror -Wno-ignored-qualifiers)

pico_add_extra_outputs(picologanal)

