// src/core/Map.hpp — 2D 网格地图（Wolf3D 风格，常量数据，无堆）
// 供 DDA 逐格查询；越界按墙处理。
// 地图数据由 scripts/mapgen.py 从 tools/maps/*.map 生成，见 generated/MapData.hpp。
#pragma once

#include <cstdint>

#include "generated/MapData.hpp"

namespace ray
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
    static constexpr int kWidth = kMapWidth;
    static constexpr int kHeight = kMapHeight;

    static constexpr WallType tileAt(int x, int y)
    {
        if (x < 0 or x >= kMapWidth or y < 0 or y >= kMapHeight)
        {
            return WallType::Brick;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        return static_cast<WallType>(kTiles[y][x]);
    }

    static constexpr bool isWall(int x, int y)
    {
        return tileAt(x, y) != WallType::Empty;
    }
};

} // namespace ray
