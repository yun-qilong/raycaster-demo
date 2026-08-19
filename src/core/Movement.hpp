// src/core/Movement.hpp — 移动逻辑（输入 -> 位移/旋转）
#pragma once

#include "core/Collision.hpp"
#include "core/Fixed.hpp"
#include "core/Mat2x2.hpp"
#include "core/Player.hpp"
#include "platform/api/Platform.hpp"

namespace ray
{

namespace Movement
{

constexpr Fixed kMoveSpeed = Fixed::fromRaw(0x1000);   // 0.0625 per tick
constexpr Fixed kKeyTurnStep = Fixed::fromRaw(0x0800); // ~0.031 rad/tick (~1.8°)
constexpr int32_t kPointerTurnScale = 200;             // mouse pixels → raw angle scaling

inline void rotate(Player &p, Fixed angle)
{
    Mat2x2 rot(angle);
    rot.mulVecInPlace(p.cam_.dir_);
    rot.mulVecInPlace(p.cam_.plane_);
}

inline void update(Player &p, const InputState &input)
{
    Fixed forward = input.moveY * kMoveSpeed;
    Fixed strafe = input.moveX * kMoveSpeed;

    FixedVec2 delta = p.cam_.dir_.scale(forward) + p.cam_.dir_.perp().scale(strafe);

    if (Collision::canMove(p.cam_.pos_.x_ + delta.x_, p.cam_.pos_.y_, p.radius_))
    {
        p.cam_.pos_.x_ = p.cam_.pos_.x_ + delta.x_;
    }
    if (Collision::canMove(p.cam_.pos_.x_, p.cam_.pos_.y_ + delta.y_, p.radius_))
    {
        p.cam_.pos_.y_ = p.cam_.pos_.y_ + delta.y_;
    }

    int keyTurn = 0;
    if (input.heldMask & ACTION_TURN_RIGHT)
    {
        keyTurn = keyTurn - 1;
    }
    if (input.heldMask & ACTION_TURN_LEFT)
    {
        keyTurn = keyTurn + 1;
    }

    int32_t totalTurnRaw = (keyTurn * kKeyTurnStep.raw()) + (input.turnDelta * kPointerTurnScale);
    Fixed angle = Fixed::fromRaw(totalTurnRaw);
    rotate(p, angle);
}

} // namespace Movement

} // namespace ray
