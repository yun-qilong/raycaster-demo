// Core/Src/game_main.cpp — MCU 侧游戏入口：缓冲存储 + 平台组装
#include "core/Color.hpp"
#include "core/Fixed.hpp"
#include "core/FixedVec2.hpp"
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

std::optional<ray::McuPlatform> g_platform;

void uartText(const char *text)
{
    HAL_UART_Transmit(&huart1, reinterpret_cast<const uint8_t *>(text),
                      static_cast<uint16_t>(strlen(text)), HAL_MAX_DELAY);
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
    uartText("game frame test: " __DATE__ " " __TIME__ "\r\n");

    g_platform.emplace(&hspi1, kDcPin, kCsPin, kRstPin, g_lcdFrameBuffer);

    ray::Player player = makePlayer();
    ray::renderFrame(player.cam_, g_gameFrameBuffer, ray::McuPlatform::kWidth,
                     ray::McuPlatform::kHeight);
    uartText("frame rendered\r\n");

    g_platform->drawBuffer(g_gameFrameBuffer);
    uartText("frame sent\r\n");

    while (true)
    {
    }
}
