#pragma once

#include "st7735.h"

class Display {
public:
  Display(Config::Display const &config);

  void draw_signals(Buffer const &data);
  void draw_rate(uint32_t rate);

  constexpr static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (r & 0x1f) << 11 | (g & 0x3f) << 5 | (b & 0x1f);
  }

private:
  void draw_glyph(uint x, uint y, uint32_t glyph, uint16_t color);

  constexpr static uint const channels = 2;

  uint const signal_height = 10;

  uint16_t const color[channels]{0xffe0, 0xf81f};
  uint8_t const vertical_offset[channels]{20, 50};

  St7735 lcd_;
};
