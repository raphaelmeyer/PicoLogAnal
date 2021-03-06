#pragma once

#include <hardware/spi.h>
#include <hardware/structs/pio.h>

#include <array>

using Buffer = std::array<uint32_t, 128>;

struct Config {

  struct Display {
    uint const clock;
    uint const mosi;
    uint const chip_select;
    uint const data_command;
    uint const reset;

    spi_inst_t *const spi;
  };

  struct Input {
    uint const probe_base;

    pio_hw_t *const pio;
  };

  struct Control {
    uint const left;
    uint const right;
  };

  struct AutoTest {
    uint const output_a;
    uint const output_b;
  };

  struct Error {
    uint const light;
  };

  Display display;
  Input input;
  Control control;
  AutoTest test;
  Error error;
};
