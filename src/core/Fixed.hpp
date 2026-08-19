// src/core/Fixed.hpp — 16.16 定点数（可移植红线：不用 float）
// 1 = 0x00010000；范围约 [-32768, 32767.99998]
#pragma once

#include <cstdint>

namespace ray
{

class Fixed
{
  public:
    using Raw = int32_t;

    constexpr Fixed() = default;

    static constexpr Fixed fromRaw(Raw raw)
    {
        Fixed f;
        f.raw_ = raw;
        return f;
    }

    static constexpr Fixed fromInt(int32_t v)
    {
        return fromRaw(static_cast<Raw>(static_cast<uint32_t>(v) << kFracBits));
    }

    [[nodiscard]] constexpr Raw raw() const
    {
        return raw_;
    }

    constexpr Fixed operator+(Fixed o) const
    {
        return fromRaw(raw_ + o.raw_);
    }

    constexpr Fixed operator-(Fixed o) const
    {
        return fromRaw(raw_ - o.raw_);
    }

    constexpr Fixed operator-() const
    {
        return fromRaw(-raw_);
    }

    constexpr Fixed operator*(Fixed o) const
    {
        return fromRaw(static_cast<Raw>((static_cast<int64_t>(raw_) * o.raw_) >> kFracBits));
    }

    // 定点除：a<<16 / b
    constexpr Fixed operator/(Fixed o) const
    {
        return fromRaw(static_cast<Raw>((static_cast<int64_t>(raw_) << kFracBits) / o.raw_));
    }

    constexpr bool operator==(Fixed o) const
    {
        return raw_ == o.raw_;
    }

    constexpr bool operator<(Fixed o) const
    {
        return raw_ < o.raw_;
    }

    constexpr bool operator<=(Fixed o) const
    {
        return raw_ <= o.raw_;
    }

    constexpr bool operator>(Fixed o) const
    {
        return raw_ > o.raw_;
    }

    constexpr bool operator>=(Fixed o) const
    {
        return raw_ >= o.raw_;
    }

    [[nodiscard]] constexpr int32_t toInt() const
    {
        return raw_ >> kFracBits;
    }

    // 绝对值（补码取负；调用方需保证不是 INT32_MIN）
    [[nodiscard]] constexpr Fixed abs() const
    {
        return raw_ < 0 ? fromRaw(-raw_) : *this;
    }

  private:
    static constexpr int kFracBits = 16;
    Raw raw_ = 0;
};

static_assert(sizeof(Fixed) == 4, "Fixed must be 4 bytes (MCU-friendly)");

} // namespace ray
