// src/app/backroomLikeDemo/Movement.hpp — 移动逻辑（位移与旋转）
#pragma once

#include "algos/Fixed.hpp"
#include "algos/Mat2x2.hpp"
#include "app/backroomLikeDemo/Collision.hpp"
#include "app/backroomLikeDemo/Map.hpp"
#include "app/backroomLikeDemo/Player.hpp"

namespace onyx::app::backroomLikeDemo
{

namespace Movement
{

constexpr algos::Fixed kMoveSpeed = algos::Fixed::fromRaw(0x1000);

inline void rotate(Player &player, algos::Fixed angle)
{
    algos::Mat2x2 rot(angle);
    rot.mulVecInPlace(player.cam_.dir_);
    rot.mulVecInPlace(player.cam_.plane_);
}

inline void applyStep(const Map &map, Player &player, algos::Fixed forward, algos::Fixed strafe,
                      algos::Fixed turn)
{
    const algos::FixedVec2 delta =
        player.cam_.dir_.scale(forward) + player.cam_.dir_.perp().scale(strafe);
    const algos::Fixed nextX = player.cam_.pos_.x_ + delta.x_;
    const algos::Fixed nextY = player.cam_.pos_.y_ + delta.y_;

    if (Collision::canMove(map, nextX, player.cam_.pos_.y_, player.radius_))
    {
        player.cam_.pos_.x_ = nextX;
    }
    if (Collision::canMove(map, player.cam_.pos_.x_, nextY, player.radius_))
    {
        player.cam_.pos_.y_ = nextY;
    }

    rotate(player, turn);
}

} // namespace Movement

} // namespace onyx::app::backroomLikeDemo
