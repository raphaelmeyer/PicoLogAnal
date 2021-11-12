#include "font.h"

#include <array>
#include <initializer_list>

namespace {

constexpr size_t const N = 4;
constexpr size_t const M = 5;
using Glyph = std::array<uint8_t, M>;

constexpr uint32_t encode_glyph(Glyph glyph) {
  uint32_t letter = 0;
  for (auto it = glyph.crbegin(); it != glyph.crend(); ++it) {
    auto row = *it;
    for (uint i = 0; i < N; ++i) {
      letter <<= 1;
      letter |= row & 1;
      row >>= 1;
    }
  }
  return letter;
}

template <typename... Glyphs> constexpr auto encode(Glyphs... glyphs) {
  return std::array{encode_glyph(glyphs)...};
}

constexpr auto const font = encode(

    Glyph{
        0b0110, //
        0b1001, //
        0b1001, //
        0b1001, //
        0b0110  //
    },
    Glyph{
        0b0010, //
        0b0010, //
        0b0010, //
        0b0010, //
        0b0010  //
    },
    Glyph{
        0b0110, //
        0b1001, //
        0b0010, //
        0b0100, //
        0b1111  //
    },
    Glyph{
        0b1111, //
        0b0001, //
        0b0111, //
        0b0001, //
        0b1111  //
    },
    Glyph{
        0b1000, //
        0b1010, //
        0b1111, //
        0b0010, //
        0b0010  //
    },
    Glyph{
        0b1111, //
        0b1000, //
        0b1110, //
        0b0001, //
        0b1110  //
    },
    Glyph{
        0b1000, //
        0b1000, //
        0b1111, //
        0b1001, //
        0b1111  //
    },
    Glyph{
        0b1111, //
        0b0001, //
        0b0010, //
        0b0100, //
        0b0100  //
    },
    Glyph{
        0b0110, //
        0b1001, //
        0b0110, //
        0b1001, //
        0b0110  //
    },
    Glyph{
        0b1111, //
        0b1001, //
        0b1111, //
        0b0001, //
        0b0001  //
    }

);

} // namespace

namespace Font {

uint32_t number_as_bitmap(uint value) {
  if (value < 10) {
    return font[value];
  }
  return 0;
}

} // namespace Font
