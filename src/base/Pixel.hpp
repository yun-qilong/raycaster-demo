#pragma once

#include <cstdint>

namespace onyx::base
{

struct Color
{
    uint8_t r_;
    uint8_t g_;
    uint8_t b_;
};

static_assert(sizeof(Color) == 3, "Color must be tightly packed 3 bytes");

struct Rgb888
{
    uint8_t r_;
    uint8_t g_;
    uint8_t b_;

    [[nodiscard]] static constexpr Rgb888 fromColor(Color color) noexcept
    {
        return Rgb888{color.r_, color.g_, color.b_};
    }
};

struct Rgb565
{
    uint16_t bits_;

    [[nodiscard]] static constexpr Rgb565 fromColor(Color color) noexcept
    {
        const auto red5 = static_cast<uint16_t>(color.r_ >> 3U);
        const auto green6 = static_cast<uint16_t>(color.g_ >> 2U);
        const auto blue5 = static_cast<uint16_t>(color.b_ >> 3U);
        return Rgb565{static_cast<uint16_t>((red5 << 11U) | (green6 << 5U) | blue5)};
    }
};

static_assert(sizeof(Rgb888) == 3, "Rgb888 must match SDL RGB24 packing");
static_assert(sizeof(Rgb565) == 2, "Rgb565 must be exactly two bytes");

} // namespace onyx::base
