#pragma once

#include "hw_config.h"

class TestSignal {
public:
  TestSignal(Config::AutoTest const &config);

  void start() const;

private:
  Config::AutoTest const config_;
};
