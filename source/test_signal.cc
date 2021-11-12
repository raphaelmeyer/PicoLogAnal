#include "test_signal.h"

#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>

TestSignal::TestSignal(Config::AutoTest const &config) : config_{config} {}

void TestSignal::start() const {
  auto const slice_a = pwm_gpio_to_slice_num(config_.output_a);
  auto const slice_b = pwm_gpio_to_slice_num(config_.output_b);

  auto const channel_a = pwm_gpio_to_channel(config_.output_a);
  auto const channel_b = pwm_gpio_to_channel(config_.output_b);

  auto pwm_config = pwm_get_default_config();
  pwm_config_set_clkdiv_int(&pwm_config, 125);
  pwm_config_set_wrap(&pwm_config, 31);

  pwm_init(slice_a, &pwm_config, false);
  if (slice_a != slice_b) {
    pwm_init(slice_b, &pwm_config, false);
  }

  pwm_set_chan_level(slice_a, channel_a, 8);
  pwm_set_chan_level(slice_b, channel_b, 16);

  gpio_set_function(config_.output_a, GPIO_FUNC_PWM);
  gpio_set_function(config_.output_b, GPIO_FUNC_PWM);

  pwm_set_enabled(slice_a, true);
  if (slice_a != slice_b) {
    pwm_set_enabled(slice_b, true);
  }
}
