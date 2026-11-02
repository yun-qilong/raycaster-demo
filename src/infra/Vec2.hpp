#pragma once

namespace onyx::infra
{

template <typename T>
struct Vec2
{
    T x_ = T{};
    T y_ = T{};

    constexpr Vec2() = default;

    constexpr Vec2(T x, T y) : x_(x), y_(y) {}
};

} // namespace onyx::infra
