#include "pico_log_anal.h"

#include "capture.h"
#include "control.h"
#include "display.h"
#include "error.h"
#include "test_signal.h"

#include <pico/multicore.h>

#include <memory>

namespace {

constexpr std::array const sampling_rate_hz{
    1'000'000u, 500'000u, 250'000u, 125'000u, 100'000u, 50000u,
    25000u,     12500u,   10000u,   5000u,    2500u};

} // namespace

PicoLogicalAnalyser::PicoLogicalAnalyser(Config const &config)
    : config_{config} {}

void PicoLogicalAnalyser::start() {
  queue_init(&events_, sizeof(Event), 16);

  TestSignal test_signal{config_.test};
  Display display{config_.display};
  Capture capture{config_.input};
  Control control{config_.control, &events_};

  test_signal.start();
  capture.arm();

  std::size_t current_rate = 0;

  for (;;) {
    auto const next_sampling_time = make_timeout_time_ms(500);

    capture.trigger(sampling_rate_hz[current_rate], capture_buffer_);

    while (not capture.done()) {
      sleep_ms(10);
    }

    Event event{};
    if (queue_try_remove(&events_, &event)) {
      switch (event) {
      case Event::ClickLeft:
        current_rate = (current_rate < sampling_rate_hz.size() - 1)
                           ? current_rate + 1
                           : sampling_rate_hz.size() - 1;
        break;

      case Event::ClickRight:
        current_rate = (current_rate > 0) ? current_rate - 1 : 0;
        break;

      default:
        break;
      }
    }

    display.draw_signals(capture_buffer_);

    display.draw_rate(sampling_rate_hz[current_rate]);

    sleep_until(next_sampling_time);
  }
}
