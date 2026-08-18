// src/core/Color.hpp — 24-bit RGB 像素（纯数据结构，平台无感）
// 与 SDL_PIXELFORMAT_RGB24 对齐；MCU 侧由平台层内部做 RGB565 转换。
#pragma once

#include <cstdint>

namespace ray
{

struct Color
{
    uint8_t r_ = 0;
    uint8_t g_ = 0;
    uint8_t b_ = 0;
};

static_assert(sizeof(Color) == 3, "Color must be tightly packed 3 bytes");

} // namespace ray
