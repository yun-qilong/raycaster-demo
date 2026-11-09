#pragma once

#include "algos/DdaCaster.hpp"
#include "algos/Fixed.hpp"
#include "app/backroomLikeDemo/Map.hpp"
#include "app/backroomLikeDemo/Movement.hpp"
#include "app/backroomLikeDemo/Player.hpp"
#include "base/AppBase.hpp"
#include "base/Pixel.hpp"
#include "contracts/doomlikeContract/Contract.hpp"

#include <cstddef>
#include <cstdint>

namespace onyx::app::backroomLikeDemo
{

template <typename PlantT>
class Game : public base::AppBase<Game<PlantT>, PlantT, contracts::doomlike::Contract>
{
  public:
    using Base = base::AppBase<Game<PlantT>, PlantT, contracts::doomlike::Contract>;
    using FrameBuffer = typename PlantT::FrameBuffer;
    using DdaCaster = algos::DdaCaster<Map, FrameBuffer::width(), FrameBuffer::height()>;
    using Columns = typename DdaCaster::Columns;

    explicit Game(PlantT &plant) noexcept : Base(plant) {}

    const contracts::doomlike::Snapshot &advanceWorld(const contracts::doomlike::Input &input)
    {
        Movement::applyStep(map_, player_, input.moveY_ * Movement::kMoveSpeed,
                            input.moveX_ * Movement::kMoveSpeed, input.turn_);
        snapshot_ = contracts::doomlike::Snapshot{
            player_.cam_.pos_,
            player_.cam_.dir_,
            player_.cam_.plane_,
        };
        return snapshot_;
    }

    void drawFrame(const contracts::doomlike::Snapshot &snapshot, FrameBuffer &frameBuffer)
    {
        const algos::Camera camera{snapshot.pos_, snapshot.dir_, snapshot.plane_};
        DdaCaster::cast(map_, camera, columns_);

        fillBackground(frameBuffer);

        for (int x = 0; x < FrameBuffer::width(); ++x)
        {
            drawColumn(frameBuffer, x, columns_[static_cast<std::size_t>(x)]);
        }
    }

  private:
    static constexpr base::Color kCeiling{72, 72, 96};
    static constexpr base::Color kFloor{64, 48, 32};
    static constexpr base::Color kFog{32, 32, 40};

    static Player makeDefaultPlayer()
    {
        Player player;
        player.cam_.pos_ = algos::FixedVec2{
            algos::Fixed::fromInt(8) + algos::Fixed::fromInt(1) / algos::Fixed::fromInt(2),
            algos::Fixed::fromInt(5) + algos::Fixed::fromInt(1) / algos::Fixed::fromInt(2),
        };
        player.cam_.dir_ = algos::FixedVec2{algos::Fixed::fromInt(-1), algos::Fixed::fromInt(0)};
        player.cam_.plane_ = algos::FixedVec2{
            algos::Fixed::fromInt(0),
            algos::Fixed::fromInt(2) / algos::Fixed::fromInt(3),
        };
        return player;
    }

    static void fillBackground(FrameBuffer &frameBuffer)
    {
        const int horizon = FrameBuffer::height() / 2;
        frameBuffer.fillColor(0, 0, FrameBuffer::width() - 1, horizon - 1, kCeiling);
        frameBuffer.fillColor(0, horizon, FrameBuffer::width() - 1, FrameBuffer::height() - 1,
                              kFloor);
    }

    void drawColumn(FrameBuffer &frameBuffer, int x, const algos::ColumnHit &hit) const
    {
        frameBuffer.fillColor(x, hit.stripeTop_, x, hit.stripeBottom_,
                              hit.hit_ ? shadedWallColor(hit) : kFog);
    }

    [[nodiscard]] base::Color shadedWallColor(const algos::ColumnHit &hit) const
    {
        const int factor = (hit.hitSide_ == algos::WallOrientation::Horizontal) ? 150 : 255;
        return shade(wallColor(map_.tileAt(hit.grid_.x_, hit.grid_.y_)), factor);
    }

    static constexpr base::Color wallColor(WallType wall)
    {
        switch (wall)
        {
        case WallType::Brick:
            return {196, 96, 48};
        case WallType::Stone:
            return {140, 140, 150};
        default:
            return {200, 200, 200};
        }
    }

    static constexpr base::Color shade(base::Color color, int factor)
    {
        return {
            shadeChannel(color.r_, factor),
            shadeChannel(color.g_, factor),
            shadeChannel(color.b_, factor),
        };
    }

    static constexpr uint8_t shadeChannel(uint8_t value, int factor)
    {
        return static_cast<uint8_t>((value * factor) >> 8);
    }

    Map map_{};
    Player player_ = makeDefaultPlayer();
    contracts::doomlike::Snapshot snapshot_{};
    Columns columns_{};
};

} // namespace onyx::app::backroomLikeDemo
