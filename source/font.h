#pragma once

#include <pico/types.h>

namespace Font {

uint32_t number_as_bitmap(uint value);
uint32_t char_as_bitmap(char character);

} // namespace Font
