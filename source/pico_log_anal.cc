#include "pico_log_anal.h"

#include "capture.h"
#include "display.h"
#include "error.h"
#include "test_signal.h"

#include <pico/multicore.h>

#include <memory>

PicoLogicalAnalyser::PicoLogicalAnalyser(Config const &config)
    : config_{config} {}

void PicoLogicalAnalyser::start() {

  TestSignal test_signal{config_.test};
  Display display{config_.display};

  test_signal.start();

  Capture::initialize(
      std::make_unique<Capture>(config_.input, capture_buffer_));

  for (;;) {
    auto const next_sampling_time = make_timeout_time_ms(500);

    if (not multicore_fifo_push_timeout_us(1'000'000, 5'000'000)) {
      Error::show();
    }

    uint32_t result{};
    if (not multicore_fifo_pop_timeout_us(5'000'000, &result)) {
      Error::show();
    }

    if (static_cast<Capture::Result>(result) == Capture::Result::Ok) {
      display.draw_signals(capture_buffer_);
    }

    display.draw_rate(1234567890);

    sleep_until(next_sampling_time);
  }
}
