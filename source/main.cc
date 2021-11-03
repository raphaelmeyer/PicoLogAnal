#include "st7735.h"

#include "sampling.pio.h"

#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/spi.h>
#include <pico/multicore.h>

#include <cmath>

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

  uint16_t prev_interval = 1000;
  multicore_fifo_push_blocking(prev_interval);

  for (;;) {
    auto const raw_value = adc_read();

    // 12bit value, scale into range 0..32
    uint32_t const scaled = raw_value >> 7;

    uint32_t const sampling_interval = 1 << scaled;

    if (sampling_interval != prev_interval) {
      multicore_fifo_push_blocking(sampling_interval);
    }
    prev_interval = sampling_interval;
    sleep_ms(100);
  }
}

int main() {
  multicore_launch_core1(main_other_core);

  St7735 display{
      spi0,      Pin::Clock, Pin::MOSI, Pin::ChipSelect, Pin::DataCommand,
      Pin::Reset};

  auto sampling_interval = multicore_fifo_pop_blocking();
  auto current_interval = sampling_interval;

  PIO const pio = pio0;
  auto const offset = pio_add_program(pio, &sampling_program);
  auto const sm = pio_claim_unused_sm(pio, true);
  auto const div = clock_get_hz(clk_sys) / 1'000'000.0f;

  sampling_program_init(pio, sm, offset, Pin::InBase, div);

  uint16_t const color_1 = 0xffe0;
  uint16_t const color_2 = 0xf81f;

  uint8_t const offset_1 = 20;
  uint8_t const offset_2 = 50;

  uint8_t x = 0;
  bool prev_1 = false;
  bool prev_2 = false;

  pio_sm_set_enabled(pio, sm, true);

  enum class State { Idle, Sample, Wait };
  State state = State::Idle;

  int wait_count = 0;
  uint32_t sampling_count = 0;

  for (;;) {
    if (multicore_fifo_rvalid()) {
      sampling_interval = multicore_fifo_pop_blocking();
    }

    auto const value = pio_sm_get_blocking(pio, sm);

    bool const value_1 = value & 0x1;
    bool const value_2 = value & 0x2;

    if (state == State::Idle) {
      if (not prev_1 and value_1) {
        draw_signal(display, 0, offset_1, prev_1, prev_1, color_1);
        draw_signal(display, 0, offset_2, prev_2, prev_2, color_2);
        draw_signal(display, 1, offset_1, value_1, prev_1, color_1);
        draw_signal(display, 1, offset_2, value_2, prev_2, color_2);
        state = State::Sample;
        x = 2;
        current_interval = sampling_interval;
        sampling_count = 0;
      }
      prev_1 = value_1;
      prev_2 = value_2;
    } else if (state == State::Sample) {
      ++sampling_count;
      if (sampling_count >= current_interval) {
        draw_signal(display, x, offset_1, value_1, prev_1, color_1);
        draw_signal(display, x, offset_2, value_2, prev_2, color_2);

        prev_1 = value_1;
        prev_2 = value_2;

        sampling_count = 0;

        ++x;
        if (x >= 128) {
          x = 0;
          state = State::Wait;
          wait_count = 1000;
        }
      }
    } else if (state == State::Wait) {
      if (wait_count > 0) {
        --wait_count;
      } else {
        state = State::Idle;
        prev_1 = value_1;
        prev_2 = value_2;
      }
    }
  }
}
