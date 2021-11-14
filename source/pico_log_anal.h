#pragma once

#include "hw_config.h"

class PicoLogicalAnalyser {
public:
  PicoLogicalAnalyser(Config const &config);

  void start();

private:
  Config const config_;
  Buffer capture_buffer_{};
};