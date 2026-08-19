// src/core/Mat2x2.hpp — 2×2 矩阵（列向量存储，定点数）
#pragma once

#include "core/Fixed.hpp"
#include "core/FixedVec2.hpp"
#include "core/Trig.hpp"
#include "core/Vec2.hpp"

namespace ray
{

class Mat2x2
{
  public:
    constexpr Mat2x2() : col0_{Fixed::fromInt(1), Fixed{}}, col1_{Fixed{}, Fixed::fromInt(1)} {}

    constexpr Mat2x2(FixedVec2 c0, FixedVec2 c1) : col0_{c0.x_, c0.y_}, col1_{c1.x_, c1.y_} {}

    explicit Mat2x2(Fixed angle)
        : col0_{Trig::cos(angle), Trig::sin(angle)}, col1_{-Trig::sin(angle), Trig::cos(angle)}
    {
    }

    void rotate(Fixed angle)
    {
        Fixed c = Trig::cos(angle);
        Fixed s = Trig::sin(angle);
        Fixed newC0x = col0_.x_ * c - col0_.y_ * s;
        Fixed newC0y = col0_.x_ * s + col0_.y_ * c;
        Fixed newC1x = col1_.x_ * c - col1_.y_ * s;
        Fixed newC1y = col1_.x_ * s + col1_.y_ * c;
        col0_ = {newC0x, newC0y};
        col1_ = {newC1x, newC1y};
    }

    [[nodiscard]] constexpr FixedVec2 col0() const
    {
        return {col0_.x_, col0_.y_};
    }
    [[nodiscard]] constexpr FixedVec2 col1() const
    {
        return {col1_.x_, col1_.y_};
    }

    [[nodiscard]] constexpr FixedVec2 mulVec(FixedVec2 v) const
    {
        return {col0_.x_ * v.x_ + col1_.x_ * v.y_, col0_.y_ * v.x_ + col1_.y_ * v.y_};
    }

    void mulVecInPlace(FixedVec2 &v) const
    {
        Fixed newX = col0_.x_ * v.x_ + col1_.x_ * v.y_;
        Fixed newY = col0_.y_ * v.x_ + col1_.y_ * v.y_;
        v.x_ = newX;
        v.y_ = newY;
    }

    [[nodiscard]] constexpr Mat2x2 mul(Mat2x2 o) const
    {
        Mat2x2 r{};
        r.col0_ = {col0_.x_ * o.col0_.x_ + col1_.x_ * o.col0_.y_,
                   col0_.y_ * o.col0_.x_ + col1_.y_ * o.col0_.y_};
        r.col1_ = {col0_.x_ * o.col1_.x_ + col1_.x_ * o.col1_.y_,
                   col0_.y_ * o.col1_.x_ + col1_.y_ * o.col1_.y_};
        return r;
    }

    [[nodiscard]] constexpr Mat2x2 transposed() const
    {
        Mat2x2 r{};
        r.col0_ = {col0_.x_, col0_.y_};
        r.col1_ = {col1_.x_, col1_.y_};
        return r;
    }

    [[nodiscard]] constexpr Fixed det() const
    {
        return col0_.x_ * col1_.y_ - col1_.x_ * col0_.y_;
    }

    [[nodiscard]] constexpr bool tryInverse(Mat2x2 &out) const
    {
        Fixed d = det();
        if (d == Fixed{})
        {
            return false;
        }
        Fixed invDet = Fixed::fromInt(1) / d;
        out = {{col1_.y_ * invDet, -col0_.y_ * invDet}, {-col1_.x_ * invDet, col0_.x_ * invDet}};
        return true;
    }

  private:
    Vec2<Fixed> col0_;
    Vec2<Fixed> col1_;
};

} // namespace ray
