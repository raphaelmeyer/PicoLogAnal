#include "st7735.h"

#include "sampling.pio.h"

#include <hardware/clocks.h>
#include <hardware/spi.h>

namespace Pin {
constexpr const uint Clock = 18;
constexpr const uint MOSI = 19;
constexpr const uint ChipSelect = 17;

constexpr const uint DataCommand = 20;
constexpr const uint Reset = 21;

constexpr const uint InBase = 14;
}; // namespace Pin

namespace {
void draw_signal(St7735 &display, uint8_t pos, uint8_t offset, bool state,
                 bool prev, uint16_t color) {
  display.set_window(pos, offset, 1, 10);
  if (state != prev) {
    for (uint i = 0; i < 10; ++i) {
      display.colorize_next_pixel(color);
    }
  } else {
    if (state) {
      display.colorize_next_pixel(color);
    }
    for (uint i = 0; i < 9; ++i) {
      display.colorize_next_pixel(0x0000);
    }
    if (not state) {
      display.colorize_next_pixel(color);
    }
  }
}
} // namespace

int main() {
  St7735 display{
      spi0,      Pin::Clock, Pin::MOSI, Pin::ChipSelect, Pin::DataCommand,
      Pin::Reset};

  PIO const pio = pio0;
  auto const offset = pio_add_program(pio, &sampling_program);
  auto const sm = pio_claim_unused_sm(pio, true);
  auto const div = clock_get_hz(clk_sys) / 1.0f;

  sampling_program_init(pio, sm, offset, Pin::InBase, div);

  uint8_t x = 0;
  bool prev_1 = false;
  bool prev_2 = false;

  pio_sm_set_enabled(pio, sm, true);

  for (;;) {
    auto const value = pio_sm_get_blocking(pio, sm);

    bool const value_1 = value & 0x1;
    bool const value_2 = value & 0x2;

    draw_signal(display, x, 20, value_1, prev_1, 0xffe0);
    draw_signal(display, x, 50, value_2, prev_2, 0xf81f);

    prev_1 = value_1;
    prev_2 = value_2;

    ++x;
    if (x >= 128) {
      x = 0;
    }
  }
}
