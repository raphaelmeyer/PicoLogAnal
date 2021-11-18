#pragma once

#include "hw_config.h"

#include <pico/util/queue.h>

class PicoLogicalAnalyser {
public:
  PicoLogicalAnalyser(Config const &config);

  void start();

private:
  Config const config_;
  queue_t events_{};
  Buffer capture_buffer_{};
};