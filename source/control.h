#pragma once

#include "events.h"
#include "hw_config.h"

#include <pico/time.h>
#include <pico/util/queue.h>

class Control {
public:
  Control(Config::Control config, queue_t *events);
  ~Control();

private:
  struct Button {
    bool pressed{false};
    absolute_time_t when{};
    Event const event;
  };

  void handle(uint gpio, uint32_t events);
  void evaluate(Button &button, bool pressed);

  Config::Control const config_;
  queue_t *events_;

  Button left_{};
  Button right_{};

  static void gpio_irq_handler(uint gpio, uint32_t events);
  static Control *self_;
};
