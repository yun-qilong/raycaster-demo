// tests/TestDdaCaster.cpp — DDA 投射的几何结果测试
#include "algos/DdaCaster.hpp"

#include <gtest/gtest.h>

namespace
{

using onyx::algos::Camera;
using onyx::algos::Fixed;
using onyx::algos::FixedVec2;
using onyx::algos::WallOrientation;

constexpr int kWidth = 320;
constexpr int kHeight = 240;
constexpr int kCenterColumn = kWidth / 2;

constexpr Fixed kHalf = Fixed::fromInt(1) / Fixed::fromInt(2);
constexpr Fixed kOneAndHalf = Fixed::fromInt(1) + kHalf;
constexpr Fixed kTwoThirds = Fixed::fromInt(2) / Fixed::fromInt(3);

constexpr FixedVec2 kEye{kOneAndHalf, kOneAndHalf};

class TestMap
{
  public:
    static constexpr int kWidth = 8;
    static constexpr int kHeight = 8;

    [[nodiscard]] constexpr bool isWall(int x, int y) const
    {
        return x == 3 or x < 0 or x >= kWidth or y < 0 or y >= kHeight;
    }
};

using Caster = onyx::algos::DdaCaster<TestMap, kWidth, kHeight>;
using NearCaster = onyx::algos::DdaCaster<TestMap, kWidth, kHeight, 1>;

} // namespace

TEST(DdaCasterTest, HitsVerticalWall)
{
    const TestMap map;
    const Camera camera{
        kEye, {Fixed::fromInt(1), Fixed::fromInt(0)}, {Fixed::fromInt(0), kTwoThirds}};

    Caster::Columns columns{};
    Caster::cast(map, camera, columns);

    const auto &hit = columns[kCenterColumn];

    EXPECT_TRUE(hit.hit_);
    EXPECT_EQ(hit.grid_.x_, 3);
    EXPECT_EQ(hit.grid_.y_, 1);
    EXPECT_EQ(hit.hitSide_, WallOrientation::Vertical);
    EXPECT_EQ(hit.distance_.raw(), kOneAndHalf.raw());
    EXPECT_EQ(hit.wallX_.raw(), kHalf.raw());
    EXPECT_EQ(hit.stripeTop_, 40);
    EXPECT_EQ(hit.stripeBottom_, 200);
}

TEST(DdaCasterTest, HitsHorizontalWall)
{
    const TestMap map;
    const Camera camera{
        kEye, {Fixed::fromInt(0), Fixed::fromInt(1)}, {-kTwoThirds, Fixed::fromInt(0)}};

    Caster::Columns columns{};
    Caster::cast(map, camera, columns);

    const auto &hit = columns[kCenterColumn];

    EXPECT_TRUE(hit.hit_);
    EXPECT_EQ(hit.grid_.y_, TestMap::kHeight);
    EXPECT_EQ(hit.hitSide_, WallOrientation::Horizontal);
    EXPECT_EQ(hit.distance_.raw(), (Fixed::fromInt(6) + kHalf).raw());
    EXPECT_EQ(hit.wallX_.raw(), kHalf.raw());
    EXPECT_LT(hit.stripeTop_, hit.stripeBottom_);
}

TEST(DdaCasterTest, ReportsMissBeyondViewDistance)
{
    const TestMap map;
    const Camera camera{
        kEye, {Fixed::fromInt(1), Fixed::fromInt(0)}, {Fixed::fromInt(0), kTwoThirds}};

    NearCaster::Columns columns{};
    NearCaster::cast(map, camera, columns);

    const auto &hit = columns[kCenterColumn];

    EXPECT_FALSE(hit.hit_);
    EXPECT_EQ(hit.distance_.raw(), Fixed::fromInt(1).raw());
    EXPECT_EQ(hit.stripeTop_, 0);
    EXPECT_EQ(hit.stripeBottom_, kHeight - 1);
}

TEST(DdaCasterTest, WallXStaysWithinOneTile)
{
    const TestMap map;
    const Camera camera{
        kEye, {Fixed::fromInt(1), Fixed::fromInt(0)}, {Fixed::fromInt(0), kTwoThirds}};

    Caster::Columns columns{};
    Caster::cast(map, camera, columns);

    for (const auto &hit : columns)
    {
        ASSERT_TRUE(hit.hit_);
        EXPECT_GE(hit.wallX_.raw(), 0);
        EXPECT_LT(hit.wallX_.raw(), Fixed::fromInt(1).raw());
    }
}
