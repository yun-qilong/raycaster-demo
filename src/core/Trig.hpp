// src/core/Trig.hpp — 定点数三角函数查表（sin/cos，256 步，无浮点）
// 精度：256 步 = 每步 1.4°，对 raycaster 场景旋转足够。
#pragma once

#include "core/Fixed.hpp"

#include <cstdint>

namespace ray
{

namespace Trig
{

namespace detail
{

// sin 查表：257 步覆盖 [0, π/2]；raw 值 = sin(angle) × 65536
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
constexpr int32_t kSinTable[257] = {
    0,     402,   804,   1206,  1608,  2010,  2412,  2814,  3216,  3617,  4019,  4420,  4821,
    5222,  5623,  6023,  6424,  6824,  7224,  7623,  8022,  8421,  8820,  9218,  9616,  10014,
    10411, 10808, 11204, 11600, 11996, 12391, 12785, 13180, 13573, 13966, 14359, 14751, 15143,
    15534, 15924, 16314, 16703, 17091, 17479, 17867, 18253, 18639, 19024, 19409, 19792, 20175,
    20557, 20939, 21320, 21699, 22078, 22457, 22834, 23210, 23586, 23961, 24335, 24708, 25080,
    25451, 25821, 26190, 26558, 26925, 27291, 27656, 28020, 28383, 28745, 29106, 29466, 29824,
    30182, 30538, 30893, 31248, 31600, 31952, 32303, 32652, 33000, 33347, 33692, 34037, 34380,
    34721, 35062, 35401, 35738, 36075, 36410, 36744, 37076, 37407, 37736, 38064, 38391, 38716,
    39040, 39362, 39683, 40002, 40320, 40636, 40951, 41264, 41576, 41886, 42194, 42501, 42806,
    43110, 43412, 43713, 44011, 44308, 44604, 44898, 45190, 45480, 45769, 46056, 46341, 46624,
    46906, 47186, 47464, 47741, 48015, 48288, 48559, 48828, 49095, 49361, 49624, 49886, 50146,
    50404, 50660, 50914, 51166, 51417, 51665, 51911, 52156, 52398, 52639, 52878, 53114, 53349,
    53581, 53812, 54040, 54267, 54491, 54714, 54934, 55152, 55368, 55582, 55794, 56004, 56212,
    56418, 56621, 56823, 57022, 57219, 57414, 57607, 57798, 57986, 58172, 58356, 58538, 58718,
    58896, 59071, 59244, 59415, 59583, 59750, 59914, 60075, 60235, 60392, 60547, 60700, 60851,
    60999, 61145, 61288, 61429, 61568, 61705, 61839, 61971, 62101, 62228, 62353, 62476, 62596,
    62714, 62830, 62943, 63054, 63162, 63268, 63372, 63473, 63572, 63668, 63763, 63854, 63944,
    64031, 64115, 64197, 64277, 64354, 64429, 64501, 64571, 64639, 64704, 64766, 64827, 64884,
    64940, 64993, 65043, 65091, 65137, 65180, 65220, 65259, 65294, 65328, 65358, 65387, 65413,
    65436, 65457, 65476, 65492, 65505, 65516, 65525, 65531, 65535, 65536,
};

constexpr int32_t kTableSize = 256;
constexpr int32_t kQuarterMask = kTableSize - 1;

} // namespace detail

// 将任意角度归约到第一象限 [0, π/2)，返回是否取反
inline bool reduceToFirstQuadrant(Fixed &angle)
{
    constexpr Fixed kTwoPi = Fixed::fromRaw(0x0006487F);
    constexpr Fixed kPi = Fixed::fromRaw(0x0003243F);
    constexpr Fixed kHalfPi = Fixed::fromRaw(0x00019220);

    bool negative = false;
    if (angle < Fixed::fromInt(0))
    {
        negative = true;
        angle = Fixed::fromInt(0) - angle;
    }
    angle = Fixed::fromRaw(angle.raw() % kTwoPi.raw());
    if (angle < kHalfPi)
    {
    }
    else if (angle < kPi)
    {
        angle = kPi - angle;
    }
    else if (angle < kPi + kHalfPi)
    {
        angle = angle - kPi;
        negative = not negative;
    }
    else
    {
        angle = kTwoPi - angle;
        negative = not negative;
    }
    return negative;
}

// sin(angle)：angle 为 16.16 定点弧度值
// 利用 sin 查表 + 对称性：sin(x) = sin(π - x) = -sin(x - π) = -sin(2π - x)
inline Fixed sin(Fixed angle)
{
    constexpr Fixed kHalfPi = Fixed::fromRaw(0x00019220);

    bool negative = reduceToFirstQuadrant(angle);

    // 线性插值查表
    Fixed index = angle * Fixed::fromInt(detail::kTableSize) / kHalfPi;
    int idx = index.toInt();
    if (idx >= detail::kTableSize)
    {
        idx = detail::kTableSize - 1;
    }
    Fixed frac = index - Fixed::fromInt(idx);

    int32_t v0 = detail::kSinTable[idx];
    int32_t v1 = detail::kSinTable[idx + 1];
    int32_t result = v0 + static_cast<int32_t>((static_cast<int64_t>(v1 - v0) * frac.raw()) >> 16);

    return negative ? Fixed::fromRaw(-result) : Fixed::fromRaw(result);
}

inline Fixed cos(Fixed angle)
{
    constexpr Fixed kHalfPi = Fixed::fromRaw(0x00019220);
    return sin(angle + kHalfPi);
}

} // namespace Trig

} // namespace ray
