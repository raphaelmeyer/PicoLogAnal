#pragma once

#include "hw_config.h"

#include <hardware/pio.h>

class Sampling {
public:
  Sampling(Config::Input config);

  int initialize();

  void prepare(uint sampling_rate_hz);
  void start();
  void stop();

private:
  Config::Input const config_;

  uint32_t const system_clock_hz_;

  pio_sm_config sm_config_{};
  uint offset_{};
  int sm_{};
};
