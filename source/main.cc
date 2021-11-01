#include "st7735.h"

#include <hardware/spi.h>

namespace Pin {
constexpr const uint Clock = 18;
constexpr const uint MOSI = 19;
constexpr const uint ChipSelect = 17;

constexpr const uint DataCommand = 20;
constexpr const uint Reset = 21;
}; // namespace Pin

int main() {
  St7735 display{
      spi0,      Pin::Clock, Pin::MOSI, Pin::ChipSelect, Pin::DataCommand,
      Pin::Reset};
}
