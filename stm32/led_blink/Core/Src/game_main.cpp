// Core/Src/game_main.cpp — MCU 侧游戏入口：缓冲存储 + 平台组装
#include "core/Color.hpp"
#include "platform/mcu/McuPlatform.hpp"

namespace
{

[[maybe_unused]] __attribute__((section(".lcdFrameBuffer"), aligned(32)))
ray::McuPlatform::DualFrameBuffer g_lcdFrameBuffer;

[[maybe_unused]] __attribute__((section(".gameFrameBuffer"), aligned(32)))
ray::Color g_gameFrameBuffer[ray::McuPlatform::kWidth * ray::McuPlatform::kHeight];

} // namespace
