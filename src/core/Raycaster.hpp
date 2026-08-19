// 逐列发射射线，DDA 求墙距，画垂直墙条带。全定点数，无堆，零平台依赖。
#pragma once

#include "core/Color.hpp"
#include "core/Fixed.hpp"
#include "core/FixedVec2.hpp"
#include "core/Map.hpp"
#include "core/Vec2.hpp"

#include <algorithm>
#include <cstdint>

namespace ray
{

enum class WallOrientation : uint8_t
{
    Horizontal,
    Vertical,
};

using IntVec2 = Vec2<int>;

struct Camera
{
    FixedVec2 pos_;
    FixedVec2 dir_;
    FixedVec2 plane_;
};

namespace detail
{

struct ColumnHit
{
    WallType wall_ = WallType::Empty;
    WallOrientation wallOrientation_ = WallOrientation::Vertical;
    int stripeTop_ = 0;
    int stripeBottom_ = 0;
};

constexpr uint8_t shade(uint8_t value, int factor);
inline void fillBackground(Color *pixels, int width, int height);
inline FixedVec2 computeUnitDirProjectionRay(const Camera &cam, int x, int width);
inline FixedVec2 computeDeltaDist(const FixedVec2 &rayUnit);
inline void initDdaStep(IntVec2 &gridStep, FixedVec2 &distToNextBoundary, const FixedVec2 &rayUnit,
                        const FixedVec2 &camPos, const IntVec2 &marchGrid,
                        const FixedVec2 &distPerGrid);
inline WallOrientation marchUntilHit(IntVec2 &marchGrid, FixedVec2 &distToNextBoundary,
                                     const FixedVec2 &distPerGrid, const IntVec2 &gridStep);
inline ColumnHit buildColumnHit(const IntVec2 &marchGrid, WallOrientation wallOrientHit,
                                Fixed perpendicularDist, int height);
inline Fixed computePerpendicularDist(WallOrientation wallOrientHit,
                                      const FixedVec2 &distToNextBoundary,
                                      const FixedVec2 &distPerGrid);
inline void drawWallStripe(Color *pixels, int width, int x, const ColumnHit &hit);

inline ColumnHit ddaCastColumn(const Camera &cam, int x, int width, int height)
{
    const FixedVec2 rayUnit = computeUnitDirProjectionRay(cam, x, width);
    const FixedVec2 distPerGrid = computeDeltaDist(rayUnit);
    IntVec2 marchGrid(cam.pos_.x_.toInt(), cam.pos_.y_.toInt());

    IntVec2 gridStep{};
    FixedVec2 distToNextBoundary{};
    initDdaStep(gridStep, distToNextBoundary, rayUnit, cam.pos_, marchGrid, distPerGrid);

    const WallOrientation wallOrientHit =
        marchUntilHit(marchGrid, distToNextBoundary, distPerGrid, gridStep);

    Fixed perpendicularDist =
        computePerpendicularDist(wallOrientHit, distToNextBoundary, distPerGrid);

    constexpr Fixed kMinDist = Fixed::fromRaw(1); // 1/65536 ≈ 0.000015 格，防除零
    if (perpendicularDist < kMinDist)
    {
        perpendicularDist = kMinDist;
    }

    return buildColumnHit(marchGrid, wallOrientHit, perpendicularDist, height);
}

constexpr uint8_t shade(uint8_t value, int factor)
{
    return static_cast<uint8_t>((value * factor) >> 8);
}

inline void fillBackground(Color *pixels, int width, int height)
{
    constexpr Color kCeiling{72, 72, 96};
    constexpr Color kFloor{64, 48, 32};
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (int y = 0; y < height; ++y)
    {
        const Color fill = (y < height / 2) ? kCeiling : kFloor;
        for (int x = 0; x < width; ++x)
        {
            pixels[(y * width) + x] = fill;
        }
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

inline FixedVec2 computeUnitDirProjectionRay(const Camera &cam, int x, int width)
{
    constexpr Fixed kOne = Fixed::fromInt(1);
    const Fixed viewOffset = Fixed::fromInt(2 * x) / Fixed::fromInt(width) - kOne;
    return FixedVec2{
        cam.dir_.x_ + cam.plane_.x_ * viewOffset,
        cam.dir_.y_ + cam.plane_.y_ * viewOffset,
    };
}

inline FixedVec2 computeDeltaDist(const FixedVec2 &rayUnit)
{
    constexpr Fixed kOne = Fixed::fromInt(1);
    constexpr Fixed kInfinity = Fixed::fromRaw(0x3FFFFFFF);

    return FixedVec2{
        (rayUnit.x_.raw() == 0) ? kInfinity : (kOne / rayUnit.x_).abs(),
        (rayUnit.y_.raw() == 0) ? kInfinity : (kOne / rayUnit.y_).abs(),
    };
}

inline void initDdaStep(IntVec2 &gridStep, FixedVec2 &distToNextBoundary, const FixedVec2 &rayUnit,
                        const FixedVec2 &camPos, const IntVec2 &marchGrid,
                        const FixedVec2 &distPerGrid)
{
    constexpr Fixed kZero = Fixed::fromInt(0);

    if (rayUnit.x_ < kZero)
    {
        gridStep.x_ = -1;
        distToNextBoundary.x_ = (camPos.x_ - Fixed::fromInt(marchGrid.x_)) * distPerGrid.x_;
    }
    else
    {
        gridStep.x_ = 1;
        distToNextBoundary.x_ = (Fixed::fromInt(marchGrid.x_ + 1) - camPos.x_) * distPerGrid.x_;
    }

    if (rayUnit.y_ < kZero)
    {
        gridStep.y_ = -1;
        distToNextBoundary.y_ = (camPos.y_ - Fixed::fromInt(marchGrid.y_)) * distPerGrid.y_;
    }
    else
    {
        gridStep.y_ = 1;
        distToNextBoundary.y_ = (Fixed::fromInt(marchGrid.y_ + 1) - camPos.y_) * distPerGrid.y_;
    }
}

inline WallOrientation marchUntilHit(IntVec2 &marchGrid, FixedVec2 &distToNextBoundary,
                                     const FixedVec2 &distPerGrid, const IntVec2 &gridStep)
{
    constexpr int kMaxDdaSteps = Map::kWidth + Map::kHeight + 8;

    bool hit = false;
    WallOrientation wallOrientation = WallOrientation::Vertical;

    for (int step = 0; step < kMaxDdaSteps; ++step)
    {
        if (distToNextBoundary.x_ < distToNextBoundary.y_)
        {
            distToNextBoundary.x_ = distToNextBoundary.x_ + distPerGrid.x_;
            marchGrid.x_ += gridStep.x_;
            wallOrientation = WallOrientation::Vertical;
        }
        else
        {
            distToNextBoundary.y_ = distToNextBoundary.y_ + distPerGrid.y_;
            marchGrid.y_ += gridStep.y_;
            wallOrientation = WallOrientation::Horizontal;
        }
        hit = Map::isWall(marchGrid.x_, marchGrid.y_);

        if (hit)
        {
            return wallOrientation;
        }
    }

    return wallOrientation;
}

inline ColumnHit buildColumnHit(const IntVec2 &marchGrid, WallOrientation wallOrientHit,
                                Fixed perpendicularDist, int height)
{
    const int wallHeightPx = (Fixed::fromInt(height) / perpendicularDist).toInt();
    ColumnHit out{};
    out.wall_ = Map::tileAt(marchGrid.x_, marchGrid.y_);
    out.wallOrientation_ = wallOrientHit;
    out.stripeTop_ = std::max((-wallHeightPx / 2) + (height / 2), 0);
    out.stripeBottom_ = std::min((wallHeightPx / 2) + (height / 2), height - 1);
    return out;
}

inline Fixed computePerpendicularDist(WallOrientation wallOrientHit,
                                      const FixedVec2 &distToNextBoundary,
                                      const FixedVec2 &distPerGrid)
{
    return (wallOrientHit == WallOrientation::Horizontal)
               ? (distToNextBoundary.y_ - distPerGrid.y_)
               : (distToNextBoundary.x_ - distPerGrid.x_);
}

inline void drawWallStripe(Color *pixels, int width, int x, const ColumnHit &hit)
{
    Color base{};
    switch (hit.wall_)
    {
    case WallType::Brick:
        base = Color{196, 96, 48};
        break;
    case WallType::Stone:
        base = Color{140, 140, 150};
        break;
    default:
        base = Color{200, 200, 200};
        break;
    }
    const int factor = (hit.wallOrientation_ == WallOrientation::Horizontal) ? 150 : 255;
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (int y = hit.stripeTop_; y <= hit.stripeBottom_; ++y)
    {
        Color &px = pixels[(y * width) + x];
        px.r_ = shade(base.r_, factor);
        px.g_ = shade(base.g_, factor);
        px.b_ = shade(base.b_, factor);
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

} // namespace detail

inline void renderFrame(const Camera &cam, Color *pixels, int width, int height)
{
    detail::fillBackground(pixels, width, height);
    for (int x = 0; x < width; ++x)
    {
        const detail::ColumnHit hit = detail::ddaCastColumn(cam, x, width, height);
        detail::drawWallStripe(pixels, width, x, hit);
    }
}

} // namespace ray
