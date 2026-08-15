// src/platform/api/Platform.hpp — 平台接口（时间/输入/显示）
// core 只依赖此接口，零平台宏。SDL（PC）与未来 MCU 后端各自实现。
#pragma once

#include <cstdint>

namespace ray
{

// 24-bit RGB 像素（3 字节，与 SDL_PIXELFORMAT_RGB24 对齐）
struct Color
{
    uint8_t r_ = 0;
    uint8_t g_ = 0;
    uint8_t b_ = 0;
};

static_assert(sizeof(Color) == 3, "Color must be tightly packed 3 bytes");

// 每 tick 采样的输入状态（键位 = 状态取最新；开火等事件后续再扩）
struct InputState
{
    bool quit_ = false;        // 请求退出（关窗口 / ESC）
    bool forward_ = false;     // W
    bool back_ = false;        // S
    bool strafeLeft_ = false;  // A
    bool strafeRight_ = false; // D
    bool turnLeft_ = false;    // ←
    bool turnRight_ = false;   // →
};

class Platform
{
  public:
    virtual ~Platform() = default;

    virtual uint32_t getTicks() = 0;                  // 毫秒时钟
    virtual InputState readInput() = 0;               // 当前按键状态
    virtual void drawBuffer(const Color *pixels) = 0; // 整帧像素（全屏）
    [[nodiscard]] virtual int screenWidth() const = 0;
    [[nodiscard]] virtual int screenHeight() const = 0;
};

} // namespace ray
