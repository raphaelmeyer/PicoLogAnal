#pragma once

#include "hw_config.h"

#include <optional>

class Error {
public:
  Error(Config::Error config);

  static void show();

private:
  static std::optional<uint> light_;
};
