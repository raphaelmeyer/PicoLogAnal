#include "capture.h"

#include "error.h"
#include "events.h"

#include "sampling.pio.h"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <pico/multicore.h>

#include <memory>

namespace {
struct Trigger {
  uint sampling_rate_hz;
  Buffer *capture_buffer;
};

class PioProgram {
public:
  PioProgram(Config::Input config, queue_t *trigger_queue, queue_t *events);

  void static main();

private:
  Config::Input const config_;
  queue_t *trigger_queue_;
  queue_t *events_;

  void idle();
};

std::unique_ptr<PioProgram> program{nullptr};

} // namespace

Capture::Capture(Config::Input config, queue_t *events)
    : config_{config}, events_{events} {}

void Capture::arm() {
  queue_init(&trigger_queue_, sizeof(Trigger), 1);

  program = std::make_unique<PioProgram>(config_, &trigger_queue_, events_);

  multicore_launch_core1(&PioProgram::main);
}

void Capture::trigger(uint sampling_rate_hz, Buffer &capture_buffer) {
  Trigger const trigger{sampling_rate_hz, &capture_buffer};
  if (not queue_try_add(&trigger_queue_, &trigger)) {
    Error::fail();
  }
}

PioProgram::PioProgram(Config::Input config, queue_t *trigger_queue,
                       queue_t *events)
    : config_{config}, trigger_queue_{trigger_queue}, events_{events} {}

void PioProgram::main() {
  Error::require(program != nullptr);
  auto self = std::move(program);
  program = nullptr;

  self->idle();
}

void PioProgram::idle() {
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
    Trigger trigger{};
    queue_remove_blocking(trigger_queue_, &trigger);

    auto const clock_div = system_clock_hz / trigger.sampling_rate_hz;

    sm_config_set_clkdiv_int_frac(&sm_config, clock_div, 0);

    pio_sm_init(config_.pio, sm, offset, &sm_config);

    dma_channel_configure(dma_channel, &dma_config,
                          trigger.capture_buffer->data(), &config_.pio->rxf[sm],
                          trigger.capture_buffer->size(), true);

    pio_sm_set_enabled(config_.pio, sm, true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    pio_sm_set_enabled(config_.pio, sm, false);

    auto const data_ready = Event::DataReady;
    queue_add_blocking(events_, &data_ready);
  }
}
