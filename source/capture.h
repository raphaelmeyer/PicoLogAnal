#pragma once

#include "hw_config.h"

#include <pico/sem.h>
#include <pico/util/queue.h>

class Capture {
public:
  Capture(Config::Input config);

  void arm();
  void trigger(uint sampling_rate_hz, Buffer &capture_buffer);

  bool done();

private:
  Config::Input const config_;
  queue_t trigger_queue_{};
  semaphore_t done_{};
};
