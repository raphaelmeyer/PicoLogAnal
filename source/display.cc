#include "display.h"

Display::Display(Config::Display const &config) : lcd_{config} {
  lcd_.clear_screen(0x0000);
}

void Display::draw_signals(Buffer const &data) {
  for (uint channel = 0; channel < 2; ++channel) {
    uint const mask = 1 << channel;
    bool previous = data.front() & mask;
    uint8_t x = 0;
    for (auto const value : data) {
      bool const state = value & mask;
      lcd_.set_window(x, vertical_offset[channel], 1, signal_height);

      if (state != previous) {
        for (uint i = 0; i < 10; ++i) {
          lcd_.colorize_next_pixel(color[channel]);
        }
      } else {
        if (state) {
          lcd_.colorize_next_pixel(color[channel]);
        }
        for (uint i = 0; i < 9; ++i) {
          lcd_.colorize_next_pixel(0x0000);
        }
        if (not state) {
          lcd_.colorize_next_pixel(color[channel]);
        }
      }

      previous = state;
      ++x;
    }
  }
}
