#pragma once

#include <cstdint>

namespace onyx::base
{

struct Rgb888
{
    uint8_t r_;
    uint8_t g_;
    uint8_t b_;

    [[nodiscard]] static constexpr Rgb888 fromRgb(uint8_t red, uint8_t green, uint8_t blue) noexcept
    {
        return Rgb888{red, green, blue};
    }
};

struct Rgb565
{
    uint16_t bits_;

    [[nodiscard]] static constexpr Rgb565 fromRgb(uint8_t red, uint8_t green, uint8_t blue) noexcept
    {
        const uint16_t red5 = static_cast<uint16_t>(red >> 3u);
        const uint16_t green6 = static_cast<uint16_t>(green >> 2u);
        const uint16_t blue5 = static_cast<uint16_t>(blue >> 3u);
        return Rgb565{static_cast<uint16_t>((red5 << 11u) | (green6 << 5u) | blue5)};
    }
};

} // namespace onyx::base
