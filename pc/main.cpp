// pc/main.cpp — 游戏主循环（固定时间步长、输入、更新、渲染）
// 数据流注入：平台调用只在此文件，core（Map/Raycaster）零平台依赖。
#include "core/Fixed.hpp"
#include "core/Movement.hpp"
#include "core/Player.hpp"
#include "core/Raycaster.hpp"
#include "platform/api/Platform.hpp"
#include "platform/sdl/SdlPlatform.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>

namespace
{

constexpr int kScreenWidth = 640;
constexpr int kScreenHeight = 400;
constexpr uint32_t kTickMs = 16;           // 60Hz 固定时间步长
constexpr uint32_t kMaxAccumulator = 1000; // 防止断点调试后循环爆炸
constexpr int kFrameSize = kScreenWidth * kScreenHeight;
constexpr uint32_t kDebugLogInterval = 60; // 每60帧输出一次调试日志

using Frame = std::array<ray::Color, kFrameSize>;

} // namespace

int main()
{
    ray::SdlPlatform platform(kScreenWidth, kScreenHeight);

    // 打开调试日志文件
    FILE *debugLog = fopen("logs/debug.log", "w");
    if (debugLog == nullptr)
    {
        return 1;
    }

    // 初始化玩家
    ray::Player player;
    player.cam_.pos_ = ray::FixedVec2{
        ray::Fixed::fromInt(8) + ray::Fixed::fromInt(1) / ray::Fixed::fromInt(2),
        ray::Fixed::fromInt(5) + ray::Fixed::fromInt(1) / ray::Fixed::fromInt(2),
    };
    player.cam_.dir_ = ray::FixedVec2{
        ray::Fixed::fromInt(-1),
        ray::Fixed::fromInt(0),
    };
    player.cam_.plane_ = ray::FixedVec2{
        ray::Fixed::fromInt(0),
        ray::Fixed::fromInt(2) / ray::Fixed::fromInt(3),
    };

    static Frame frame; // 固定大小数组，无堆

    uint32_t accumulator = 0;
    uint32_t previousTime = platform.getTicks();
    uint32_t frameCount = 0;

    // 游戏主循环
    while (true)
    {
        uint32_t currentTime = platform.getTicks();
        uint32_t elapsed = currentTime - previousTime;
        previousTime = currentTime;

        elapsed = std::min(elapsed, kMaxAccumulator);

        accumulator += elapsed;

        // 固定时间步长更新
        while (accumulator >= kTickMs)
        {
            ray::InputState input;
            platform.sampleInput(input);

            if (input.quit)
            {
                fclose(debugLog);
                return 0;
            }

            // 更新玩家状态
            ray::Movement::update(player, input);

            accumulator -= kTickMs;
        }

        // 渲染帧
        ray::renderFrame(player.cam_, frame.data(), kScreenWidth, kScreenHeight);
        platform.drawBuffer(frame.data());

        frameCount++;

        // 定期输出调试信息
        if (frameCount % kDebugLogInterval == 0)
        {
            fprintf(debugLog, "frame=%u pos=(%d,%d) dir=(%d,%d) elapsed=%u\n", frameCount,
                    player.cam_.pos_.x_.raw(), player.cam_.pos_.y_.raw(), player.cam_.dir_.x_.raw(),
                    player.cam_.dir_.y_.raw(), elapsed);
            fflush(debugLog);
        }
    }

    fclose(debugLog);
    return 0;
}
