#pragma once

#include <pico/types.h>

#include <initializer_list>

extern "C" struct spi_inst;

class St7735 {
public:
  St7735(spi_inst *spi_hw, uint clock_pin, uint mosi_pin, uint chip_select_pin,
         uint data_command_pin, uint reset_pin);

  void set_window(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
  void colorize_next_pixel(uint16_t color);

  void clear_screen(uint16_t color);

private:
  void write_command(uint8_t command);
  void write_command(uint8_t command,
                     std::initializer_list<uint8_t> const &data);

  spi_inst *hw_;

  uint const clock_;
  uint const mosi_;
  uint const chip_select_;
  uint const data_command_;
  uint const reset_;
};
