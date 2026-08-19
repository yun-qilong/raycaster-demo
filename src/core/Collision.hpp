// src/core/Collision.hpp — 碰撞检测（AABB vs 网格）
#pragma once

#include "core/Fixed.hpp"
#include "core/Map.hpp"

namespace ray
{

namespace Collision
{

inline int floorToInt(Fixed v)
{
    return v.raw() >> 16;
}

inline int ceilToInt(Fixed v)
{
    return (v.raw() + 0xFFFF) >> 16;
}

inline bool isOutOfBounds(int x0, int y0, int x1, int y1)
{
    return (x0 < 0) or (y0 < 0) or (x1 >= Map::kWidth) or (y1 >= Map::kHeight);
}

inline bool isAreaBlocked(int x0, int y0, int x1, int y1)
{
    for (int ty = y0; ty <= y1; ++ty)
    {
        for (int tx = x0; tx <= x1; ++tx)
        {
            if (Map::isWall(tx, ty))
            {
                return true;
            }
        }
    }
    return false;
}

inline bool canMove(Fixed x, Fixed y, Fixed radius)
{
    int x0 = floorToInt(x - radius);
    int y0 = floorToInt(y - radius);
    int x1 = ceilToInt(x + radius) - 1;
    int y1 = ceilToInt(y + radius) - 1;

    if (isOutOfBounds(x0, y0, x1, y1))
    {
        return false;
    }
    return not isAreaBlocked(x0, y0, x1, y1);
}

} // namespace Collision

} // namespace ray
