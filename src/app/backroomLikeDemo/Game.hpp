// src/app/backroomLikeDemo/Game.hpp — 本游戏：世界推进与画面绘制
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

    template <typename FrameBufferT>
    void drawFrame(const contracts::doomlike::Snapshot &snapshot, FrameBufferT &frameBuffer)
    {
        const algos::Camera camera{snapshot.pos_, snapshot.dir_, snapshot.plane_};
        DdaCaster::cast(map_, camera, columns_);

        fillBackground(frameBuffer);

        for (int x = 0; x < FrameBufferT::width(); ++x)
        {
            drawColumn(frameBuffer, x, columns_[static_cast<std::size_t>(x)]);
        }
    }

  private:
    static constexpr base::Rgb888 kCeiling{72, 72, 96};
    static constexpr base::Rgb888 kFloor{64, 48, 32};
    static constexpr base::Rgb888 kFog{32, 32, 40};

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

    template <typename FrameBufferT>
    static void fillBackground(FrameBufferT &frameBuffer)
    {
        for (int y = 0; y < FrameBufferT::height(); ++y)
        {
            const base::Rgb888 color = (y < FrameBufferT::height() / 2) ? kCeiling : kFloor;

            for (int x = 0; x < FrameBufferT::width(); ++x)
            {
                frameBuffer.pixelAt(x, y) = color;
            }
        }
    }

    template <typename FrameBufferT>
    void drawColumn(FrameBufferT &frameBuffer, int x, const algos::ColumnHit &hit) const
    {
        const base::Rgb888 color = hit.hit_ ? shadedWallColor(hit) : kFog;

        for (int y = hit.stripeTop_; y <= hit.stripeBottom_; ++y)
        {
            frameBuffer.pixelAt(x, y) = color;
        }
    }

    [[nodiscard]] base::Rgb888 shadedWallColor(const algos::ColumnHit &hit) const
    {
        const int factor = (hit.hitSide_ == algos::WallOrientation::Horizontal) ? 150 : 255;
        const base::Rgb888 base = wallColor(map_.tileAt(hit.grid_.x_, hit.grid_.y_));

        return {
            shadeChannel(base.r_, factor),
            shadeChannel(base.g_, factor),
            shadeChannel(base.b_, factor),
        };
    }

    static constexpr base::Rgb888 wallColor(WallType wall)
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
