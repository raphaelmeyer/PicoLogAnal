#include "pico_log_anal.h"

#include "display.h"

#include "sampling.pio.h"

#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/multicore.h>

#include <array>

void main_other_core() {
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

PicoLogicalAnalyser::PicoLogicalAnalyser(Config const &config)
    : config_{config} {}

void PicoLogicalAnalyser::start() {

  adc_init();
  adc_gpio_init(config_.control.scaler);
  multicore_launch_core1(main_other_core);

  Display display{config_.display};

  // auto sampling_interval = multicore_fifo_pop_blocking();
  // auto current_interval = sampling_interval;

  auto const offset = pio_add_program(config_.input.pio, &sampling_program);
  auto const sm = pio_claim_unused_sm(config_.input.pio, true);
  auto const div = clock_get_hz(clk_sys) / 1'000'000.0f;

  sampling_program_init(config_.input.pio, sm, offset, config_.input.probe_base,
                        div);

  auto dma_channel = dma_claim_unused_channel(true);
  auto dma_config = dma_channel_get_default_config(dma_channel);
  channel_config_set_read_increment(&dma_config, false);
  channel_config_set_write_increment(&dma_config, true);
  channel_config_set_dreq(&dma_config,
                          pio_get_dreq(config_.input.pio, sm, false));

  Buffer capture_buffer{};

  // sampling rate ? change PIO clock divider ?

  for (;;) {
    pio_sm_set_enabled(config_.input.pio, sm, false);
    pio_sm_clear_fifos(config_.input.pio, sm);
    pio_sm_restart(config_.input.pio, sm);

    dma_channel_configure(dma_channel, &dma_config, capture_buffer.data(),
                          &config_.input.pio->rxf[sm], capture_buffer.size(),
                          true);

    auto const next_sampling_time = make_timeout_time_ms(500);

    pio_sm_set_enabled(config_.input.pio, sm, true);
    dma_channel_wait_for_finish_blocking(dma_channel);

    display.draw_signals(capture_buffer);

    sleep_until(next_sampling_time);
  }
}
