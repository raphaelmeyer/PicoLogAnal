#pragma once

#include "hw_config.h"

#include <memory>

class Capture {
public:
  Capture(Config::Input config, Buffer &capture_buffer);

  static void initialize(std::unique_ptr<Capture> self);

  enum class Result : uint32_t { Error, Ok };

private:
  void idle();
  Config::Input const config_;
  Buffer &capture_buffer_;

  void static capture_main();
  static std::unique_ptr<Capture> self_;
};
