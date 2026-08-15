// src/main.cpp — hello window：SDL 窗口 + 像素填充
// 首提交目标：打通工具链（CMake+SDL2）与平台抽象，再迭代渲染。
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

// 填一帧渐变色（临时验证 drawBuffer 通路）
void fillFrame(Frame &buffer)
{
    for (int y = 0; y < kScreenHeight; ++y)
    {
        for (int x = 0; x < kScreenWidth; ++x)
        {
            const int idx = y * kScreenWidth + x;
            buffer[idx].r_ = static_cast<uint8_t>((x * 255) / (kScreenWidth - 1));
            buffer[idx].g_ = static_cast<uint8_t>((y * 255) / (kScreenHeight - 1));
            buffer[idx].b_ =
                static_cast<uint8_t>(((x + y) * 255) / (kScreenWidth + kScreenHeight - 2));
        }
    }
}

} // namespace

int main()
{
    ray::SdlPlatform platform(kScreenWidth, kScreenHeight);

    static Frame frame; // 固定大小数组，无堆
    fillFrame(frame);
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
