#pragma once

#include "events.h"
#include "hw_config.h"

#include <pico/util/queue.h>

class PicoLogicalAnalyser {
public:
  PicoLogicalAnalyser(Config const &config);

  void start();

private:
  bool schedule_event(Event event);
  static int64_t trigger_capture(alarm_id_t id, void *user_data);

  Config const config_;
  queue_t events_{};
  Buffer capture_buffer_{};
};