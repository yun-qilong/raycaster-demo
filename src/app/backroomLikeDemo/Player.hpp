// src/app/backroomLikeDemo/Player.hpp — 玩家状态（位置、方向、半径）
#pragma once

#include "algos/DdaCaster.hpp"
#include "algos/Fixed.hpp"

namespace onyx::app::backroomLikeDemo
{

struct Player
{
    algos::Camera cam_;
    algos::Fixed radius_ = algos::Fixed::fromRaw(0x3333);
};

} // namespace onyx::app::backroomLikeDemo
