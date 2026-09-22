// Core/Src/game_main.cpp — MCU 侧游戏入口：缓冲存储 + 平台组装
#include "core/Color.hpp"
#include "core/Fixed.hpp"
#include "core/FixedVec2.hpp"
#include "core/GameLoop.hpp"
#include "core/Player.hpp"
#include "core/Raycaster.hpp"
#include "platform/mcu/McuPlatform.hpp"

#include "main.h"

#include <cstring>
#include <optional>

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

namespace
{

__attribute__((section(".lcdFrameBuffer"), aligned(32)))
ray::McuPlatform::DualFrameBuffer g_lcdFrameBuffer;

__attribute__((section(".gameFrameBuffer"), aligned(32)))
ray::Color g_gameFrameBuffer[ray::McuPlatform::kWidth * ray::McuPlatform::kHeight];

const GpioPin kDcPin{GPIOB, GPIO_PIN_0};
const GpioPin kCsPin{GPIOA, GPIO_PIN_4};
const GpioPin kRstPin{GPIOB, GPIO_PIN_1};

constexpr uint32_t kHeartbeatMs = 5000;

std::optional<ray::McuPlatform> g_platform;

void uartText(const char *text)
{
    HAL_UART_Transmit(&huart1, reinterpret_cast<const uint8_t *>(text),
                      static_cast<uint16_t>(strlen(text)), HAL_MAX_DELAY);
}

void uartNumber(uint32_t value)
{
    char digits[12] = {};
    int pos = 11;
    do
    {
        digits[--pos] = static_cast<char>('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U && pos > 0);

    HAL_UART_Transmit(&huart1, reinterpret_cast<const uint8_t *>(&digits[pos]),
                      static_cast<uint16_t>(11 - pos), HAL_MAX_DELAY);
}

ray::Player makePlayer()
{
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
    return player;
}

} // namespace

extern "C" void gameMain(void)
{
    uartText("game loop test: " __DATE__ " " __TIME__ "\r\n");

    g_platform.emplace(&hspi1, kDcPin, kCsPin, kRstPin, g_lcdFrameBuffer);

    ray::Player player = makePlayer();

    ray::GameLoop::run(*g_platform, player, g_gameFrameBuffer,
                       [](uint32_t, uint32_t, const ray::Player &)
                       {
                           static uint32_t lastReport = 0;
                           static uint32_t lastRendered = 0;
                           static uint32_t lastPushed = 0;

                           const uint32_t now = HAL_GetTick();
                           if ((now - lastReport) < kHeartbeatMs)
                           {
                               return;
                           }

                           const uint32_t rendered = g_platform->renderedFrames();
                           const uint32_t pushed = g_platform->pushedFrames();
                           lastReport = now;

                           uartText("rendered=");
                           uartNumber(rendered - lastRendered);
                           uartText(" pushed=");
                           uartNumber(pushed - lastPushed);
                           uartText("\r\n");

                           lastRendered = rendered;
                           lastPushed = pushed;
                       });
}
