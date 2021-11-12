#include "pico_log_anal.h"

#include "display.h"
#include "test_signal.h"

#include "sampling.pio.h"

#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/multicore.h>

#include <array>

void main_other_core() {
  adc_select_input(0);

  uint16_t prev_interval = 10000;
  multicore_fifo_push_blocking(prev_interval);

  for (;;) {
    auto const raw_value = adc_read();

    uint32_t const sampling_interval =
        std::max(2, std::min(raw_value / 170, 22)) - 2;

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

  // auto const system_clock_hz = clock_get_hz(clk_sys);

  TestSignal test_signal{config_.test};

  adc_init();
  adc_gpio_init(config_.control.scaler);
  multicore_launch_core1(main_other_core);

  Display display{config_.display};

  auto sampling_interval = multicore_fifo_pop_blocking();

  auto const offset = pio_add_program(config_.input.pio, &sampling_program);
  auto const sm = pio_claim_unused_sm(config_.input.pio, true);

  auto sm_config = sampling_program_get_default_config(offset);
  sampling_program_init(config_.input.pio, sm, &sm_config,
                        config_.input.probe_base);

  test_signal.start();

  auto dma_channel = dma_claim_unused_channel(true);
  auto dma_config = dma_channel_get_default_config(dma_channel);
  channel_config_set_read_increment(&dma_config, false);
  channel_config_set_write_increment(&dma_config, true);
  channel_config_set_dreq(&dma_config,
                          pio_get_dreq(config_.input.pio, sm, false));

  Buffer capture_buffer{};

  for (;;) {
    auto const div = 125;

    sm_config_set_clkdiv_int_frac(&sm_config, div, 0);

    pio_sm_init(config_.input.pio, sm, offset, &sm_config);

    dma_channel_configure(dma_channel, &dma_config, capture_buffer.data(),
                          &config_.input.pio->rxf[sm], capture_buffer.size(),
                          true);

    auto const next_sampling_time = make_timeout_time_ms(500);

    pio_sm_set_enabled(config_.input.pio, sm, true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    pio_sm_set_enabled(config_.input.pio, sm, false);

    display.draw_signals(capture_buffer);

    display.draw_rate(sampling_interval);

    while (multicore_fifo_rvalid()) {
      sampling_interval = multicore_fifo_pop_blocking();
    }

    sleep_until(next_sampling_time);
  }
}
