#include "pico_log_anal.h"

#include "display.h"

#include "sampling.pio.h"

#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/multicore.h>

#include <hardware/pwm.h>

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

  auto const system_clock_hz = clock_get_hz(clk_sys);

  adc_init();
  adc_gpio_init(config_.control.scaler);
  multicore_launch_core1(main_other_core);

  Display display{config_.display};

  // auto sampling_interval = multicore_fifo_pop_blocking();
  // auto current_interval = sampling_interval;

  auto const offset = pio_add_program(config_.input.pio, &sampling_program);
  auto const sm = pio_claim_unused_sm(config_.input.pio, true);
  auto const div = system_clock_hz / 1'000'000.0f;

  auto sm_config = sampling_program_get_default_config(offset);
  sampling_program_init(config_.input.pio, sm, &sm_config,
                        config_.input.probe_base, div);

  // ----------------------------------
  // auto test signal
  uint const autotest_a = 0;
  uint const autotest_b = 2;

  auto const slice_a = pwm_gpio_to_slice_num(autotest_a);
  auto const slice_b = pwm_gpio_to_slice_num(autotest_b);
  auto const channel_a = pwm_gpio_to_channel(autotest_a);
  auto const channel_b = pwm_gpio_to_channel(autotest_b);

  auto pwm_config = pwm_get_default_config();
  pwm_config_set_clkdiv(&pwm_config, system_clock_hz / 250'000.0f);
  pwm_config_set_wrap(&pwm_config, 10);

  pwm_init(slice_a, &pwm_config, false);
  pwm_init(slice_b, &pwm_config, false);

  pwm_set_chan_level(slice_a, channel_a, 4);
  pwm_set_chan_level(slice_b, channel_b, 7);

  gpio_set_function(autotest_a, GPIO_FUNC_PWM);
  gpio_set_function(autotest_b, GPIO_FUNC_PWM);

  pwm_set_enabled(slice_a, true);
  pwm_set_enabled(slice_b, true);
  // ----------------------------------

  auto dma_channel = dma_claim_unused_channel(true);
  auto dma_config = dma_channel_get_default_config(dma_channel);
  channel_config_set_read_increment(&dma_config, false);
  channel_config_set_write_increment(&dma_config, true);
  channel_config_set_dreq(&dma_config,
                          pio_get_dreq(config_.input.pio, sm, false));

  Buffer capture_buffer{};

  // sampling rate ? change PIO clock divider ?

  for (;;) {
    pio_sm_init(config_.input.pio, sm, offset, &sm_config);

    dma_channel_configure(dma_channel, &dma_config, capture_buffer.data(),
                          &config_.input.pio->rxf[sm], capture_buffer.size(),
                          true);

    auto const next_sampling_time = make_timeout_time_ms(500);

    pio_sm_set_enabled(config_.input.pio, sm, true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    pio_sm_set_enabled(config_.input.pio, sm, false);

    display.draw_signals(capture_buffer);

    sleep_until(next_sampling_time);
  }
}
