#include "display.h"

Display::Display(Config::Display const &config) : lcd_{config} {
  lcd_.clear_screen(0x0000);
}

void Display::draw_signals(Buffer const &data) {
  for (uint channel = 0; channel < 2; ++channel) {
    uint const mask = 1 << channel;
    uint8_t x = 0;

    bool state = data.front() & mask;

    for (auto it = data.cbegin(); it < data.cend(); ++it) {
      bool const previous = state;
      state = *it & mask;
      bool const next =
          std::next(it) != data.cend() ? *std::next(it) & mask : state;

      lcd_.set_window(x, vertical_offset[channel], 1, signal_height);

      if (state && (not previous || not next)) {
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

      ++x;
    }
  }
}

void Display::draw_rate(uint32_t rate) {
  lcd_.set_window(8, 100, 10, 21);
  for (uint i = 0; i <= 20; ++i) {
    for (uint j = 0; j < 10; ++j) {
      if ((i == 0 || i == 20) && (j % 3 == 0)) {
        lcd_.colorize_next_pixel(rgb(0x10, 0x20, 0x10));
      } else if (i == rate) {
        lcd_.colorize_next_pixel(rgb(0x1f, 0x20, 0x00));
      } else {
        lcd_.colorize_next_pixel(0x0000);
      }
    }
  }
}
