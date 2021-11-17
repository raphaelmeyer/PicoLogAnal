#pragma once

#include "hw_config.h"

#include <optional>

class Error {
public:
  Error(Config::Error config);

  static void fail();
  static void require(bool condition);

private:
  static std::optional<uint> light_;
};
