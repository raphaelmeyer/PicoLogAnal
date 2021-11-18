#include "pico_log_anal.h"

#include "capture.h"
#include "control.h"
#include "display.h"
#include "error.h"
#include "test_signal.h"

#include <pico/multicore.h>
#include <pico/time.h>

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
  auto alarms = alarm_pool_get_default();

  TestSignal test_signal{config_.test};
  Display display{config_.display};
  Capture capture{config_.input, &events_};
  Control control{config_.control, &events_};

  test_signal.start();
  capture.arm();

  std::size_t current_rate = 0;
  display.draw_rate(sampling_rate_hz[current_rate]);

  auto next_sampling_time = make_timeout_time_ms(0);
  auto const initial_trigger = Event::TriggerCapture;
  queue_add_blocking(&events_, &initial_trigger);

  for (;;) {
    Event event{};
    if (queue_try_remove(&events_, &event)) {
      switch (event) {

      case Event::ClickLeft: {
        current_rate = (current_rate < sampling_rate_hz.size() - 1)
                           ? current_rate + 1
                           : sampling_rate_hz.size() - 1;
        display.draw_rate(sampling_rate_hz[current_rate]);
      } break;

      case Event::ClickRight: {
        current_rate = (current_rate > 0) ? current_rate - 1 : 0;
        display.draw_rate(sampling_rate_hz[current_rate]);
      } break;

      case Event::DataReady: {
        display.draw_signals(capture_buffer_);
        auto const id = alarm_pool_add_alarm_at(
            alarms, next_sampling_time, &PicoLogicalAnalyser::trigger_capture,
            this, true);
        Error::require(id >= 0);
      } break;

      case Event::TriggerCapture: {
        next_sampling_time = make_timeout_time_ms(500);
        capture.trigger(sampling_rate_hz[current_rate], capture_buffer_);
      } break;

      default:
        break;
      }
    }

    sleep_until(next_sampling_time);
  }
}

bool PicoLogicalAnalyser::schedule_event(Event event) {
  return queue_try_add(&events_, &event);
}

int64_t PicoLogicalAnalyser::trigger_capture([[maybe_unused]] alarm_id_t id,
                                             void *user_data) {
  if (user_data) {
    auto self = static_cast<PicoLogicalAnalyser *>(user_data);
    if (not self->schedule_event(Event::TriggerCapture)) {
      return 1000;
    }
  }
  return 0;
}
