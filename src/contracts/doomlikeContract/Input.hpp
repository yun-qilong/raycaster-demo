#pragma once

#include "infra/Fixed.hpp"

namespace onyx::contracts::doomlike
{

struct Input
{
    onyx::infra::Fixed moveX_ = onyx::infra::Fixed::fromInt(0);
    onyx::infra::Fixed moveY_ = onyx::infra::Fixed::fromInt(0);
    onyx::infra::Fixed turn_ = onyx::infra::Fixed::fromInt(0);
    bool quit_ = false;
};

} // namespace onyx::contracts::doomlike
