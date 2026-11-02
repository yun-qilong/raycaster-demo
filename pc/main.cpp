// pc/main.cpp — PC 入口：建对象 + 接线，主循环在 core/GameLoop.hpp
#include "core/GameLoop.hpp"
#include "infra/Fixed.hpp"
#include "infra/FixedVec2.hpp"
#include "infra/Player.hpp"
#include "platform/sdl/SdlPlatform.hpp"

#include <array>
#include <cstdint>
#include <cstdio>

namespace
{

constexpr int kScreenWidth = 640;
constexpr int kScreenHeight = 400;
constexpr int kFrameSize = kScreenWidth * kScreenHeight;
constexpr uint32_t kDebugLogInterval = 60; // 每60帧输出一次调试日志

using Frame = std::array<onyx::infra::Color, kFrameSize>;

onyx::infra::Player makePlayer()
{
    onyx::infra::Player player;
    player.cam_.pos_ = onyx::infra::FixedVec2{
        onyx::infra::Fixed::fromInt(8) +
            onyx::infra::Fixed::fromInt(1) / onyx::infra::Fixed::fromInt(2),
        onyx::infra::Fixed::fromInt(5) +
            onyx::infra::Fixed::fromInt(1) / onyx::infra::Fixed::fromInt(2),
    };
    player.cam_.dir_ = onyx::infra::FixedVec2{
        onyx::infra::Fixed::fromInt(-1),
        onyx::infra::Fixed::fromInt(0),
    };
    player.cam_.plane_ = onyx::infra::FixedVec2{
        onyx::infra::Fixed::fromInt(0),
        onyx::infra::Fixed::fromInt(2) / onyx::infra::Fixed::fromInt(3),
    };
    return player;
}

} // namespace

int main()
{
    ray::SdlPlatform platform(kScreenWidth, kScreenHeight);

    FILE *debugLog = fopen("logs/debug.log", "w");
    if (debugLog == nullptr)
    {
        return 1;
    }

    onyx::infra::Player player = makePlayer();
    static Frame frame; // 固定大小数组，无堆

    ray::GameLoop::run(
        platform, player, frame.data(),
        [&debugLog](uint32_t frameCount, uint32_t elapsed, const onyx::infra::Player &p)
        {
            if (frameCount % kDebugLogInterval != 0)
            {
                return;
            }
            fprintf(debugLog, "frame=%u pos=(%d,%d) dir=(%d,%d) elapsed=%u\n", frameCount,
                    p.cam_.pos_.x_.raw(), p.cam_.pos_.y_.raw(), p.cam_.dir_.x_.raw(),
                    p.cam_.dir_.y_.raw(), elapsed);
            fflush(debugLog);
        });

    fclose(debugLog);
    return 0;
}
