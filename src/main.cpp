// src/main.cpp — 固定位置/视角渲染一帧 2.5D 画面，显示 3 秒后退出
// 数据流注入：平台调用只在此文件，core（Map/Raycaster）零平台依赖。
#include "core/Fixed.hpp"
#include "core/Raycaster.hpp"
#include "platform/api/Platform.hpp"
#include "platform/sdl/SdlPlatform.hpp"

#include <array>
#include <cstdint>

namespace
{

constexpr int kScreenWidth = 640;
constexpr int kScreenHeight = 400;
constexpr uint32_t kRunMs = 3000; // 显示 3 秒后自动退出
constexpr int kFrameSize = kScreenWidth * kScreenHeight;

using Frame = std::array<ray::Color, kFrameSize>;

// 固定相机：位置 (8.5, 5.5)，朝 -x，FOV≈67°（相机平面 = 2/3）
const ray::Camera kCamera = {
    ray::FixedVec2{
        ray::Fixed::fromInt(8) + ray::Fixed::fromInt(1) / ray::Fixed::fromInt(2),
        ray::Fixed::fromInt(5) + ray::Fixed::fromInt(1) / ray::Fixed::fromInt(2),
    },
    ray::FixedVec2{
        ray::Fixed::fromInt(-1),
        ray::Fixed::fromInt(0),
    },
    ray::FixedVec2{
        ray::Fixed::fromInt(0),
        ray::Fixed::fromInt(2) / ray::Fixed::fromInt(3),
    },
};

} // namespace

int main()
{
    ray::SdlPlatform platform(kScreenWidth, kScreenHeight);

    static Frame frame; // 固定大小数组，无堆
    ray::renderFrame(kCamera, frame.data(), kScreenWidth, kScreenHeight);
    platform.drawBuffer(frame.data());

    const uint32_t start = platform.getTicks();
    while (platform.getTicks() - start < kRunMs)
    {
        ray::InputState input = platform.readInput();
        if (input.quit_)
        {
            break;
        }
    }
    return 0;
}
