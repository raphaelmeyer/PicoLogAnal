#include "sampling.h"
#include "sampling.pio.h"

#include <hardware/clocks.h>

Sampling::Sampling(Config::Input config)
    : config_{config}, system_clock_hz_{clock_get_hz(clk_sys)} {}

int Sampling::initialize() {
  offset_ = pio_add_program(config_.pio, &sampling_program);
  sm_ = pio_claim_unused_sm(config_.pio, true);

  sm_config_ = sampling_program_get_default_config(offset_);

  sm_config_set_in_pins(&sm_config_, config_.probe_base);
  pio_sm_set_consecutive_pindirs(config_.pio, sm_, config_.probe_base, 2,
                                 false);

  pio_gpio_init(config_.pio, config_.probe_base);
  pio_gpio_init(config_.pio, config_.probe_base + 1);

  sm_config_set_in_shift(&sm_config_, false, true, 2);
  sm_config_set_fifo_join(&sm_config_, PIO_FIFO_JOIN_RX);

  return sm_;
}

void Sampling::prepare(uint sampling_rate_hz) {
  auto const clock_div = system_clock_hz_ / sampling_rate_hz;
  sm_config_set_clkdiv_int_frac(&sm_config_, clock_div, 0);
  pio_sm_init(config_.pio, sm_, offset_, &sm_config_);
}

void Sampling::start() { pio_sm_set_enabled(config_.pio, sm_, true); }

void Sampling::stop() { pio_sm_set_enabled(config_.pio, sm_, false); }
