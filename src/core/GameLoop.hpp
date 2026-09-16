// src/core/GameLoop.hpp — 游戏主循环（平台无关；固定时间步长）
// 通过模板参数适配各平台后端（SdlPlatform / McuPlatform）；帧缓冲与调试钩子由调用方提供。
#pragma once

#include "core/Color.hpp"
#include "core/Movement.hpp"
#include "core/Player.hpp"
#include "core/Raycaster.hpp"
#include "platform/api/Platform.hpp"

#include <algorithm>
#include <cstdint>

namespace ray
{

class GameLoop
{
  public:
    template <typename PlatformT, typename OnFrameT>
    static void run(PlatformT &platform, Player &player, Color *frame, OnFrameT onFrame)
    {
        const int width = platform.screenWidth();
        const int height = platform.screenHeight();

        uint32_t accumulator = 0;
        uint32_t previousTime = platform.getTicks();
        uint32_t frameCount = 0;

        while (true)
        {
            const uint32_t elapsed = frameElapsed(platform, previousTime);

            if (not advanceSimulation(platform, player, accumulator, elapsed))
            {
                return;
            }

            presentFrame(platform, player, frame, width, height);
            ++frameCount;
            onFrame(frameCount, elapsed, player);
        }
    }

    template <typename PlatformT>
    static void run(PlatformT &platform, Player &player, Color *frame)
    {
        run(platform, player, frame, [](uint32_t, uint32_t, const Player &) {});
    }

  private:
    static constexpr uint32_t kTickMs = 16;           // 60Hz 固定时间步长
    static constexpr uint32_t kMaxAccumulator = 1000; // 防止断点调试后循环爆炸

    template <typename PlatformT>
    static uint32_t frameElapsed(PlatformT &platform, uint32_t &previousTime)
    {
        const uint32_t currentTime = platform.getTicks();
        const uint32_t elapsed = currentTime - previousTime;
        previousTime = currentTime;
        return std::min(elapsed, kMaxAccumulator);
    }

    template <typename PlatformT>
    static bool advanceSimulation(PlatformT &platform, Player &player, uint32_t &accumulator,
                                  uint32_t elapsed)
    {
        accumulator += elapsed;
        while (accumulator >= kTickMs)
        {
            InputState input;
            platform.sampleInput(input);

            if (input.quit)
            {
                return false;
            }

            Movement::update(player, input);
            accumulator -= kTickMs;
        }
        return true;
    }

    template <typename PlatformT>
    static void presentFrame(PlatformT &platform, Player &player, Color *frame, int width,
                             int height)
    {
        renderFrame(player.cam_, frame, width, height);
        platform.drawBuffer(frame);
    }
};

} // namespace ray
