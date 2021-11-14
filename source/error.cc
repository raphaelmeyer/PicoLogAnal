#include "error.h"

#include <hardware/gpio.h>

std::optional<uint> Error::light_ = std::nullopt;

Error::Error(Config::Error config) { light_ = config.light; }

void Error::show() {
  uint pin = PICO_DEFAULT_LED_PIN;
  if (light_) {
    pin = *light_;
  }

  gpio_init(pin);
  gpio_set_dir(pin, true);
  for (;;) {
    gpio_put(pin, true);
    sleep_ms(250);
    gpio_put(pin, false);
    sleep_ms(250);
  }
}
