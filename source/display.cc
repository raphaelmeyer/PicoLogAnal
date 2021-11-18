#include "display.h"

#include "font.h"

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

  uint x = 110;
  uint const y = 115;

  draw_glyph(x, y, Font::char_as_bitmap('z'), rgb(0x10, 0x20, 0x1f));
  x -= 5;

  draw_glyph(x, y, Font::char_as_bitmap('H'), rgb(0x10, 0x20, 0x1f));
  x -= 6;

  do {
    draw_glyph(x, y, Font::number_as_bitmap(rate % 10), 0xffff);
    rate /= 10;
    x -= 5;
  } while (rate > 0);

  while (x > 55) {
    lcd_.set_window(x, y, 4, 5);
    for (uint i = 0; i < 20; ++i) {
      lcd_.colorize_next_pixel(0x0000);
    }
    x -= 5;
  }
}

void Display::draw_glyph(uint x, uint y, uint32_t glyph, uint16_t color) {
  lcd_.set_window(x, y, 4, 5);

  for (uint i = 0; i < 20; ++i) {
    if (glyph & 1) {
      lcd_.colorize_next_pixel(color);
    } else {
      lcd_.colorize_next_pixel(0x0000);
    }
    glyph >>= 1;
  }
}
