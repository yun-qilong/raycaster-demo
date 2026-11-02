// src/infra/Map.hpp — 2D 网格地图（Wolf3D 风格，常量数据，无堆）
// 供 DDA 逐格查询；越界按墙处理。
// 地图数据由 scripts/mapgen.py 从 tools/maps/*.map 生成，见 generated/MapData.hpp。
#pragma once

#include <cstdint>

#include "generated/MapData.hpp"

namespace onyx::infra
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

    static constexpr WallType tileAt(int x, int y)
    {
        if (x < 0 or x >= ray::kMapWidth or y < 0 or y >= ray::kMapHeight)
        {
            return WallType::Brick;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        return static_cast<WallType>(ray::kTiles[y][x]);
    }

    static constexpr bool isWall(int x, int y)
    {
        return tileAt(x, y) != WallType::Empty;
    }
};

} // namespace onyx::infra
