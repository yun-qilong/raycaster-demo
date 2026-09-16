// tests/TestFixed.cpp — 16.16 定点数冒烟测试
#include "core/Fixed.hpp"

#include <gtest/gtest.h>

using ray::Fixed;

TEST(FixedTest, FromIntAndToInt)
{
    EXPECT_EQ(Fixed::fromInt(3).toInt(), 3);
    EXPECT_EQ(Fixed::fromInt(-2).toInt(), -2);
}

TEST(FixedTest, AddSub)
{
    const Fixed a = Fixed::fromInt(10);
    const Fixed b = Fixed::fromInt(4);
    EXPECT_EQ((a + b).toInt(), 14);
    EXPECT_EQ((a - b).toInt(), 6);
}

TEST(FixedTest, Mul)
{
    const Fixed a = Fixed::fromInt(10);
    const Fixed b = Fixed::fromInt(4);
    EXPECT_EQ((a * b).toInt(), 40);
}

TEST(FixedTest, Div)
{
    const Fixed a = Fixed::fromInt(10);
    const Fixed b = Fixed::fromInt(4);
    EXPECT_EQ((a / b).toInt(), 2); // 10/4 = 2.5 -> 截断 2
}

TEST(FixedTest, Fraction)
{
    const Fixed half = Fixed::fromRaw(0x8000);        // 0.5
    EXPECT_EQ((Fixed::fromInt(3) * half).toInt(), 1); // 1.5 -> 1
    EXPECT_EQ((half + half).toInt(), 1);
}
