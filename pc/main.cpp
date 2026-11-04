#include "app/backroomLikeDemo/Game.hpp"
#include "platform/sdl/PcPlant.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>

namespace
{

constexpr uint32_t kTickMs = 16;
constexpr uint32_t kMaxElapsed = 1000;
constexpr uint32_t kDebugLogInterval = 60;

using App = onyx::app::backroomLikeDemo::Game<onyx::platform::PcPlant>;
using Snapshot = onyx::contracts::doomlike::Snapshot;

void logFrame(FILE *log, uint32_t frameCount, uint32_t elapsed, const Snapshot *snapshot)
{
    if ((frameCount % kDebugLogInterval) != 0 or snapshot == nullptr)
    {
        return;
    }

    fprintf(log, "frame=%u pos=(%d,%d) dir=(%d,%d) elapsed=%u\n", frameCount,
            snapshot->pos_.x_.raw(), snapshot->pos_.y_.raw(), snapshot->dir_.x_.raw(),
            snapshot->dir_.y_.raw(), elapsed);
    fflush(log);
}

void runHost(App &app, onyx::platform::PcPlant &plant, FILE *log)
{
    uint32_t previousTime = plant.nowMs();
    uint32_t accumulator = 0;
    uint32_t frameCount = 0;

    while (not plant.quitRequested())
    {
        const uint32_t now = plant.nowMs();
        const uint32_t elapsed = std::min(now - previousTime, kMaxElapsed);
        previousTime = now;
        accumulator += elapsed;

        while (accumulator >= kTickMs)
        {
            app.stepLogic();
            accumulator -= kTickMs;
        }

        if (plant.quitRequested())
        {
            break;
        }

        app.presentFrame();
        ++frameCount;
        logFrame(log, frameCount, elapsed, plant.getSnapshot());
    }
}

} // namespace

int main()
{
    static onyx::platform::PcPlant plant;
    static App app(plant);

    FILE *debugLog = fopen("logs/debug.log", "w");
    if (debugLog == nullptr)
    {
        return 1;
    }

    runHost(app, plant, debugLog);
    fclose(debugLog);
    return 0;
}
