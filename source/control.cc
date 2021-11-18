#include "control.h"

#include "error.h"

#include <hardware/gpio.h>
#include <hardware/irq.h>

bool state = false;

Control *Control::self_ = nullptr;

Control::Control(Config::Control config, queue_t *events)
    : config_{config}, events_{events}, left_{.event = Event::ClickLeft},
      right_{.event = Event::ClickRight} {
  self_ = this;

  gpio_set_irq_enabled_with_callback(config_.left,
                                     GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                     true, &Control::gpio_irq_handler);

  gpio_set_irq_enabled(config_.right, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                       true);
}

Control::~Control() { self_ = nullptr; }

void Control::handle(uint gpio, [[maybe_unused]] uint32_t events) {
  bool const state = gpio_get(gpio);
  if (gpio == config_.left) {
    evaluate(left_, state);
  } else if (gpio == config_.right) {
    evaluate(right_, state);
  }
}

void Control::evaluate(Button &button, bool pressed) {
  if (button.pressed == pressed) {
    return;
  }

  auto const now = get_absolute_time();
  button.pressed = pressed;

  if (pressed) {
    button.when = now;
  } else {
    if (absolute_time_diff_us(button.when, now) > 50'000) {
      queue_try_add(events_, &button.event);
    }
  }
}

void Control::gpio_irq_handler(uint gpio, uint32_t events) {
  if (self_) {
    self_->handle(gpio, events);
  }
}
