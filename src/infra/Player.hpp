// src/infra/Player.hpp — 玩家状态（位置、方向、半径）
#pragma once

#include "infra/Fixed.hpp"
#include "infra/Raycaster.hpp"

namespace onyx::infra
{

struct Player
{
    Camera cam_;
    Fixed radius_ = Fixed::fromRaw(0x3333); // 0.2 in 16.16 fixed-point
};

} // namespace onyx::infra
