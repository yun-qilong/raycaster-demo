// src/core/FixedVec2.hpp — 2D 定点向量（向量运算，兼容 POD 访问）
#pragma once

#include "core/Fixed.hpp"
#include "core/Trig.hpp"

namespace ray
{

class FixedVec2
{
  public:
    Fixed x_{};
    Fixed y_{};

    constexpr FixedVec2() = default;
    constexpr FixedVec2(Fixed x, Fixed y) : x_(x), y_(y) {}

    constexpr FixedVec2 &operator+=(FixedVec2 o)
    {
        x_ = x_ + o.x_;
        y_ = y_ + o.y_;
        return *this;
    }

    constexpr FixedVec2 &operator-=(FixedVec2 o)
    {
        x_ = x_ - o.x_;
        y_ = y_ - o.y_;
        return *this;
    }

    [[nodiscard]] constexpr FixedVec2 operator+(FixedVec2 o) const
    {
        return {x_ + o.x_, y_ + o.y_};
    }
    [[nodiscard]] constexpr FixedVec2 operator-(FixedVec2 o) const
    {
        return {x_ - o.x_, y_ - o.y_};
    }
    [[nodiscard]] constexpr FixedVec2 operator-() const
    {
        return {-x_, -y_};
    }

    [[nodiscard]] constexpr FixedVec2 scale(Fixed s) const
    {
        return {x_ * s, y_ * s};
    }

    [[nodiscard]] constexpr Fixed dot(FixedVec2 o) const
    {
        return x_ * o.x_ + y_ * o.y_;
    }
    [[nodiscard]] constexpr Fixed cross(FixedVec2 o) const
    {
        return x_ * o.y_ - y_ * o.x_;
    }
    [[nodiscard]] constexpr FixedVec2 perp() const
    {
        return {-y_, x_};
    }

    void rotate(Fixed angle)
    {
        Fixed c = Trig::cos(angle);
        Fixed s = Trig::sin(angle);
        Fixed newX = x_ * c - y_ * s;
        Fixed newY = x_ * s + y_ * c;
        x_ = newX;
        y_ = newY;
    }

    [[nodiscard]] constexpr Fixed lengthSq() const
    {
        return dot(*this);
    }
};

} // namespace ray
