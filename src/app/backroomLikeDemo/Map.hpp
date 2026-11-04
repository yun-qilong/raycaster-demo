// src/app/backroomLikeDemo/Map.hpp — 本游戏的 2D 网格地图数据
#pragma once

#include "generated/MapData.hpp"

#include <cstdint>

namespace onyx::app::backroomLikeDemo
{

enum class WallType : uint8_t
{
    Empty = 0,
    Brick = 1,
    Stone = 2,
};

class Map
{
  public:
    static constexpr int kWidth = ray::kMapWidth;
    static constexpr int kHeight = ray::kMapHeight;

    [[nodiscard]] constexpr WallType tileAt(int x, int y) const
    {
        if (x < 0 or x >= kWidth or y < 0 or y >= kHeight)
        {
            return WallType::Brick;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        return static_cast<WallType>(ray::kTiles[y][x]);
    }

    [[nodiscard]] constexpr bool isWall(int x, int y) const
    {
        return tileAt(x, y) != WallType::Empty;
    }
};

} // namespace onyx::app::backroomLikeDemo
