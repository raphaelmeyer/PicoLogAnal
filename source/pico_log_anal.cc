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
  Capture capture{config_.input};

  test_signal.start();
  capture.arm();

  for (;;) {
    auto const next_sampling_time = make_timeout_time_ms(500);

    uint const sampling_rate_hz = 1'000'000;
    auto const result = capture.trigger(sampling_rate_hz, capture_buffer_);

    if (result == Capture::Result::Ready) {
      display.draw_signals(capture_buffer_);
    }

    display.draw_rate(sampling_rate_hz);

    sleep_until(next_sampling_time);
  }
}
