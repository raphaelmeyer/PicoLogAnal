#include "st7735.h"

#include "sampling.pio.h"

#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/spi.h>
#include <pico/multicore.h>

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

void main_other_core() {

  adc_init();
  adc_gpio_init(26);
  adc_select_input(0);

  uint16_t prev = 16;
  multicore_fifo_push_blocking(prev);

  for (;;) {
    auto const value = adc_read();

    // calibration ?
    auto const factor = value >> 7;

    // calculate window size
    // 0 -> 1us, 1 -> 2us, 2 -> 5us, 3 -> 10us, 4 -> 20us, 5 -> 50us ... 1s

    if (factor != prev) {
      multicore_fifo_push_blocking(factor);
    }
    prev = factor;
    sleep_ms(100);
  }
}

int main() {
  multicore_launch_core1(main_other_core);

  St7735 display{
      spi0,      Pin::Clock, Pin::MOSI, Pin::ChipSelect, Pin::DataCommand,
      Pin::Reset};

  auto window = multicore_fifo_pop_blocking();

  PIO const pio = pio0;
  auto const offset = pio_add_program(pio, &sampling_program);
  auto const sm = pio_claim_unused_sm(pio, true);
  auto const div = clock_get_hz(clk_sys) / 1.0f;

  sampling_program_init(pio, sm, offset, Pin::InBase, div);

  uint8_t x = 0;
  bool prev_1 = false;
  bool prev_2 = false;

  uint8_t f_x_ = 0;

  pio_sm_set_enabled(pio, sm, true);

  // analyse adc
  display.set_window(0, 79, 128, 1);
  for (uint i = 0; i < 128; ++i) {
    display.colorize_next_pixel(0x8410);
  }
  display.set_window(0, 112, 128, 1);
  for (uint i = 0; i < 128; ++i) {
    display.colorize_next_pixel(0x8410);
  }

  for (;;) {
    if (multicore_fifo_rvalid()) {
      window = multicore_fifo_pop_blocking();

      // analyse adc
      display.set_window(f_x_, 80, 1, 32);
      for (uint i = 0; i < 31; ++i) {
        display.colorize_next_pixel(window == i ? 0x07e0 : 0x0000);
      }
      display.colorize_next_pixel(window >= 32 ? 0xf800 : 0x0000);

      f_x_ = (f_x_ + 1) & 0x7f;
    }

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
