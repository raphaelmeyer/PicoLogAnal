#include "capture.h"

#include "error.h"

#include "sampling.pio.h"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <pico/multicore.h>

std::unique_ptr<Capture> Capture::self_ = nullptr;

Capture::Capture(Config::Input config, Buffer &capture_buffer)
    : config_{config}, capture_buffer_{capture_buffer} {}

void Capture::idle() {
  auto const system_clock_hz = clock_get_hz(clk_sys);

  auto const offset = pio_add_program(config_.pio, &sampling_program);
  auto const sm = pio_claim_unused_sm(config_.pio, true);

  auto sm_config = sampling_program_get_default_config(offset);
  sampling_program_init(config_.pio, sm, &sm_config, config_.probe_base);

  auto dma_channel = dma_claim_unused_channel(true);
  auto dma_config = dma_channel_get_default_config(dma_channel);
  channel_config_set_read_increment(&dma_config, false);
  channel_config_set_write_increment(&dma_config, true);
  channel_config_set_dreq(&dma_config, pio_get_dreq(config_.pio, sm, false));

  for (;;) {
    auto const sampling_rate = multicore_fifo_pop_blocking();
    auto const clock_div = system_clock_hz / sampling_rate;

    sm_config_set_clkdiv_int_frac(&sm_config, clock_div, 0);

    pio_sm_init(config_.pio, sm, offset, &sm_config);

    dma_channel_configure(dma_channel, &dma_config, capture_buffer_.data(),
                          &config_.pio->rxf[sm], capture_buffer_.size(), true);

    pio_sm_set_enabled(config_.pio, sm, true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    pio_sm_set_enabled(config_.pio, sm, false);

    multicore_fifo_push_blocking(static_cast<uint32_t>(Result::Ok));
  }
}

void Capture::initialize(std::unique_ptr<Capture> self) {
  self_ = std::move(self);
  multicore_launch_core1(&Capture::capture_main);
}

void Capture::capture_main() {
  if (not self_) {
    Error::show();
  } else {
    self_->idle();
  }
}
