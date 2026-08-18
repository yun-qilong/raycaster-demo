// src/core/Map.hpp — 2D 网格地图（Wolf3D 风格，常量数据，无堆）
// 供 DDA 逐格查询；越界按墙处理。
#pragma once

#include <array>
#include <cstdint>

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
    static constexpr int kWidth = 24;
    static constexpr int kHeight = 24;

    static constexpr WallType tileAt(int x, int y)
    {
        if (x < 0 or x >= kWidth or y < 0 or y >= kHeight)
        {
            return WallType::Brick; // 越界 = 外墙
        }
        // 运行时下标查询是 DDA 的核心用法，常量下标规则在此不适用
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        return static_cast<WallType>(kTiles[y][x]);
    }

    static constexpr bool isWall(int x, int y)
    {
        return tileAt(x, y) != WallType::Empty;
    }

  private:
    // 经典 Wolf3D 风格测试图：外墙 + 右上房间 block（南墙混入 Stone）
    // 玩家固定相机在 (8.5, 5.5) 朝 -x 看，可见 block 南墙与远端外墙。
    static constexpr std::array<std::array<uint8_t, kWidth>, kHeight> kTiles = {
        std::array<uint8_t, kWidth>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        std::array<uint8_t, kWidth>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };
};

} // namespace ray
