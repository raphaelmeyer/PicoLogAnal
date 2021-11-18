#pragma once

#include "hw_config.h"

#include <pico/util/queue.h>

class Capture {
public:
  Capture(Config::Input config, queue_t *events);

  void arm();
  void trigger(uint sampling_rate_hz, Buffer &capture_buffer);

private:
  Config::Input const config_;
  queue_t *events_;

  queue_t trigger_queue_{};
};
