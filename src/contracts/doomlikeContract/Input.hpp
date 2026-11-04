#pragma once

#include "algos/Fixed.hpp"

#include <cstdint>

namespace onyx::contracts::doomlike
{

constexpr algos::Fixed kKeyTurnStep = algos::Fixed::fromRaw(0x0800);
constexpr int32_t kPointerTurnScale = 200;

struct Input
{
    algos::Fixed moveX_ = algos::Fixed::fromInt(0);
    algos::Fixed moveY_ = algos::Fixed::fromInt(0);
    algos::Fixed turn_ = algos::Fixed::fromInt(0);
};

} // namespace onyx::contracts::doomlike
