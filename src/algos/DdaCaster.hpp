#pragma once

#include "algos/Fixed.hpp"
#include "algos/FixedVec2.hpp"
#include "algos/Vec2.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace onyx::algos
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

struct ColumnHit
{
    IntVec2 grid_{};
    WallOrientation hitSide_ = WallOrientation::Vertical;
    bool hit_ = false;
    Fixed distance_ = Fixed::fromInt(0);
    Fixed wallX_ = Fixed::fromInt(0);
    int stripeTop_ = 0;
    int stripeBottom_ = 0;
};

template <typename MapT, int WidthT, int HeightT,
          int MaxViewDistanceT = MapT::kWidth + MapT::kHeight>
class DdaCaster
{
  public:
    static_assert(WidthT > 0, "viewport width must be positive");
    static_assert(HeightT > 0, "viewport height must be positive");
    static_assert(MaxViewDistanceT > 0, "max view distance must be positive");

    using Columns = std::array<ColumnHit, static_cast<std::size_t>(WidthT)>;

    static void cast(const MapT &map, const Camera &camera, Columns &columns) noexcept
    {
        for (int x = 0; x < WidthT; ++x)
        {
            columns[static_cast<std::size_t>(x)] = castColumn(map, camera, x);
        }
    }

  private:
    [[nodiscard]] static ColumnHit castColumn(const MapT &map, const Camera &camera, int x)
    {
        const FixedVec2 rayUnit = rayForColumn(camera, x);
        const FixedVec2 distPerGrid = computeDeltaDist(rayUnit);
        IntVec2 marchGrid(camera.pos_.x_.toInt(), camera.pos_.y_.toInt());

        IntVec2 gridStep{};
        FixedVec2 distToNextBoundary{};
        initDdaStep(camera, gridStep, distToNextBoundary, rayUnit, marchGrid, distPerGrid);

        WallOrientation hitSide = WallOrientation::Vertical;

        if (not marchUntilHit(map, marchGrid, distToNextBoundary, distPerGrid, gridStep, hitSide))
        {
            return buildMiss(marchGrid);
        }

        const Fixed distance =
            clampDistance(computePerpendicularDist(hitSide, distToNextBoundary, distPerGrid));
        const Fixed wallX = computeWallX(camera, hitSide, rayUnit, distance);

        return buildHit(marchGrid, hitSide, distance, wallX);
    }

    [[nodiscard]] static FixedVec2 rayForColumn(const Camera &camera, int x)
    {
        constexpr Fixed kOne = Fixed::fromInt(1);
        const Fixed viewOffset = Fixed::fromInt(2 * x) / Fixed::fromInt(WidthT) - kOne;

        return FixedVec2{
            camera.dir_.x_ + camera.plane_.x_ * viewOffset,
            camera.dir_.y_ + camera.plane_.y_ * viewOffset,
        };
    }

    [[nodiscard]] static FixedVec2 computeDeltaDist(const FixedVec2 &rayUnit)
    {
        constexpr Fixed kOne = Fixed::fromInt(1);
        constexpr Fixed kInfinity = Fixed::fromRaw(0x3FFFFFFF);

        return FixedVec2{
            (rayUnit.x_.raw() == 0) ? kInfinity : (kOne / rayUnit.x_).abs(),
            (rayUnit.y_.raw() == 0) ? kInfinity : (kOne / rayUnit.y_).abs(),
        };
    }

    static void initDdaStep(const Camera &camera, IntVec2 &gridStep, FixedVec2 &distToNextBoundary,
                            const FixedVec2 &rayUnit, const IntVec2 &marchGrid,
                            const FixedVec2 &distPerGrid)
    {
        constexpr Fixed kZero = Fixed::fromInt(0);

        gridStep.x_ = (rayUnit.x_ < kZero) ? -1 : 1;
        gridStep.y_ = (rayUnit.y_ < kZero) ? -1 : 1;
        distToNextBoundary.x_ =
            distanceToExitBoundary(camera.pos_.x_, marchGrid.x_, gridStep.x_, distPerGrid.x_);
        distToNextBoundary.y_ =
            distanceToExitBoundary(camera.pos_.y_, marchGrid.y_, gridStep.y_, distPerGrid.y_);
    }

    [[nodiscard]] static Fixed distanceToExitBoundary(Fixed camPos, int gridPos, int gridStep,
                                                      Fixed distPerGrid)
    {
        const Fixed boundaryOffset = (gridStep < 0) ? (camPos - Fixed::fromInt(gridPos))
                                                    : (Fixed::fromInt(gridPos + 1) - camPos);
        return boundaryOffset * distPerGrid;
    }

    [[nodiscard]] static bool marchUntilHit(const MapT &map, IntVec2 &marchGrid,
                                            FixedVec2 &distToNextBoundary,
                                            const FixedVec2 &distPerGrid, const IntVec2 &gridStep,
                                            WallOrientation &hitSide)
    {
        constexpr int kMaxMarchSteps = MapT::kWidth + MapT::kHeight;

        for (int step = 0; step < kMaxMarchSteps; ++step)
        {
            if (maxViewDistance() < nextBoundaryDistance(distToNextBoundary))
            {
                return false;
            }

            hitSide = advanceStep(marchGrid, distToNextBoundary, distPerGrid, gridStep);

            if (map.isWall(marchGrid.x_, marchGrid.y_))
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] static constexpr Fixed maxViewDistance()
    {
        return Fixed::fromInt(MaxViewDistanceT);
    }

    [[nodiscard]] static Fixed nextBoundaryDistance(const FixedVec2 &distToNextBoundary)
    {
        return (distToNextBoundary.x_ < distToNextBoundary.y_) ? distToNextBoundary.x_
                                                               : distToNextBoundary.y_;
    }

    [[nodiscard]] static WallOrientation advanceStep(IntVec2 &marchGrid,
                                                     FixedVec2 &distToNextBoundary,
                                                     const FixedVec2 &distPerGrid,
                                                     const IntVec2 &gridStep)
    {
        if (distToNextBoundary.x_ < distToNextBoundary.y_)
        {
            distToNextBoundary.x_ = distToNextBoundary.x_ + distPerGrid.x_;
            marchGrid.x_ += gridStep.x_;
            return WallOrientation::Vertical;
        }

        distToNextBoundary.y_ = distToNextBoundary.y_ + distPerGrid.y_;
        marchGrid.y_ += gridStep.y_;
        return WallOrientation::Horizontal;
    }

    [[nodiscard]] static Fixed computePerpendicularDist(WallOrientation hitSide,
                                                        const FixedVec2 &distToNextBoundary,
                                                        const FixedVec2 &distPerGrid)
    {
        return (hitSide == WallOrientation::Horizontal) ? (distToNextBoundary.y_ - distPerGrid.y_)
                                                        : (distToNextBoundary.x_ - distPerGrid.x_);
    }

    [[nodiscard]] static Fixed clampDistance(Fixed distance)
    {
        constexpr Fixed kMinDist = Fixed::fromRaw(1);
        return (distance < kMinDist) ? kMinDist : distance;
    }

    [[nodiscard]] static Fixed computeWallX(const Camera &camera, WallOrientation hitSide,
                                            const FixedVec2 &rayUnit, Fixed distance)
    {
        const Fixed hitCoord = (hitSide == WallOrientation::Vertical)
                                   ? (camera.pos_.y_ + distance * rayUnit.y_)
                                   : (camera.pos_.x_ + distance * rayUnit.x_);
        return Fixed::fromRaw(hitCoord.raw() & 0xFFFF);
    }

    [[nodiscard]] static ColumnHit buildHit(const IntVec2 &marchGrid, WallOrientation hitSide,
                                            Fixed distance, Fixed wallX)
    {
        ColumnHit hit{};
        hit.grid_ = marchGrid;
        hit.hitSide_ = hitSide;
        hit.hit_ = true;
        hit.distance_ = distance;
        hit.wallX_ = wallX;
        hit.stripeTop_ = stripeTop(distance);
        hit.stripeBottom_ = stripeBottom(distance);
        return hit;
    }

    [[nodiscard]] static ColumnHit buildMiss(const IntVec2 &stopGrid)
    {
        ColumnHit hit{};
        hit.grid_ = stopGrid;
        hit.distance_ = maxViewDistance();
        hit.stripeTop_ = stripeTop(hit.distance_);
        hit.stripeBottom_ = stripeBottom(hit.distance_);
        return hit;
    }

    [[nodiscard]] static int stripeTop(Fixed distance)
    {
        return std::max((-wallHeightPx(distance) / 2) + (HeightT / 2), 0);
    }

    [[nodiscard]] static int stripeBottom(Fixed distance)
    {
        return std::min((wallHeightPx(distance) / 2) + (HeightT / 2), HeightT - 1);
    }

    [[nodiscard]] static int wallHeightPx(Fixed distance)
    {
        return (Fixed::fromInt(HeightT) / distance).toInt();
    }
};

} // namespace onyx::algos
