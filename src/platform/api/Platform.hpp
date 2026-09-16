// src/platform/api/Platform.hpp — 平台接口（时间/输入/显示）
// core 只依赖此接口，零平台宏。SDL（PC）与未来 MCU 后端各自实现。
#pragma once

#include "core/Color.hpp"
#include "core/Fixed.hpp"
#include "utils/CrtpBase.hpp"

#include <cstdint>

namespace ray
{

// 功能键掩码
constexpr uint32_t ACTION_QUIT = 1u << 0;
constexpr uint32_t ACTION_USE = 1u << 1;
constexpr uint32_t ACTION_PAUSE = 1u << 2;
constexpr uint32_t ACTION_TURN_LEFT = 1u << 3;
constexpr uint32_t ACTION_TURN_RIGHT = 1u << 4;

// 每 tick 采样的输入状态（语义输入，平台负责映射）
struct InputState
{
    Fixed moveX = Fixed::fromInt(0); // 横向移动量，范围 [-1, 1]，正为向右
    Fixed moveY = Fixed::fromInt(0); // 纵向移动量，范围 [-1, 1]，正为向前
    int32_t turnDelta = 0;           // 增量旋转，来自鼠标相对位移或旋钮计数
    uint32_t heldMask = 0;           // 当前按住的功能键位掩码
    bool quit = false;               // 请求退出（关窗口 / ESC）
};

template <typename Impl>
class Platform : public utils::CrtpBase<Impl>
{
  public:
    uint32_t getTicks()
    {
        return this->getImplementation().getTicksImpl(); // 毫秒时钟
    }

    void sampleInput(InputState &out)
    {
        this->getImplementation().sampleInputImpl(out); // 采样当前输入状态
    }

    void drawBuffer(const Color *pixels)
    {
        this->getImplementation().drawBufferImpl(pixels); // 整帧像素（全屏）
    }

    [[nodiscard]] int screenWidth() const
    {
        return this->getImplementation().screenWidthImpl();
    }

    [[nodiscard]] int screenHeight() const
    {
        return this->getImplementation().screenHeightImpl();
    }
};

} // namespace ray
