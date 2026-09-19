#include "main.h"

#include "lcdriv.hpp"

#include <cstring>
#include <optional>

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

namespace
{

// ---------------------------------------------------------------------------
// v20: v18 established the operating point. Same static picture, four clocks:
//
//    5 MHz  structural corruption (stripes, misalignment)
//   10 MHz  structural corruption
//   20 MHz  occasional wrong-colour pixels (bit errors)
//   40 MHz  STABLE
//
// 40 MHz is therefore the working point - it is also the frequency this panel
// was originally brought up at. This build locks it in and is the first image
// meant to be looked at rather than interrogated:
//
//   white background, green reference block at (40,40), red block in the middle
//   Key1 / Key2 move the red block one step left / right
//   one pushFrame per frame, CS-split framing, heartbeat on the UART
//
// On top of the library fix from v15 (command byte and parameters in separate
// CS frames) this is the configuration that produced a stable picture.
// ---------------------------------------------------------------------------

constexpr int kPanelWidth = 320;
constexpr int kPanelHeight = 240;

constexpr uint16_t kWhite = 0xFFFF;
constexpr uint16_t kBoxColor = 0xF800;
constexpr uint16_t kGreenBoxColor = 0x07E0;
constexpr int32_t kBoxSize = 40;
constexpr int32_t kBoxCenterX = (kPanelWidth - kBoxSize) / 2;
constexpr int32_t kBoxY = (kPanelHeight - kBoxSize) / 2;
constexpr int32_t kBoxStep = 100;
constexpr int32_t kBoxSpeedPx = 8;
constexpr int32_t kGreenBoxX = 40;
constexpr int32_t kGreenBoxY = 40;

using Lcd = LcdDriver<BusType::SPI, ControllerType::ILI9341, kPanelWidth, kPanelHeight>;

std::optional<Lcd> lcd;

const GpioPin kDcPin{GPIOB, GPIO_PIN_0};
const GpioPin kCsPins[1] = {{GPIOA, GPIO_PIN_4}};
const GpioPin kRstPins[1] = {{GPIOB, GPIO_PIN_1}};

__attribute__((section(".lcdFrameBuffer"), aligned(32))) uint8_t frameBuffer[Lcd::kBytes];

void uartWrite(const char *text, uint16_t length)
{
    HAL_UART_Transmit(&huart1, reinterpret_cast<const uint8_t *>(text), length, HAL_MAX_DELAY);
}

void uartText(const char *text)
{
    uartWrite(text, static_cast<uint16_t>(strlen(text)));
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
    uartWrite(&digits[pos], static_cast<uint16_t>(11 - pos));
}

void printTestStamp()
{
    const char text[] = "Test Build: " __DATE__ " " __TIME__ "\r\n";
    uartWrite(text, static_cast<uint16_t>(sizeof(text) - 1));
}

void paintRect(int32_t x, int32_t y, int32_t size, uint16_t color)
{
    const uint8_t high = static_cast<uint8_t>(color >> 8);
    const uint8_t low = static_cast<uint8_t>(color & 0xFF);
    for (int32_t row = 0; row < size; ++row)
    {
        uint8_t *pixel = &frameBuffer[((y + row) * kPanelWidth + x) * 2];
        for (int32_t i = 0; i < size; ++i)
        {
            pixel[i * 2] = high;
            pixel[i * 2 + 1] = low;
        }
    }
}

void composeFrame(int32_t boxX)
{
    memset(frameBuffer, 0xFF, sizeof(frameBuffer));
    paintRect(kGreenBoxX, kGreenBoxY, kBoxSize, kGreenBoxColor);
    paintRect(boxX, kBoxY, kBoxSize, kBoxColor);
}

bool keyPressed(GPIO_TypeDef *port, uint16_t pin)
{
    return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
}

int32_t resolveTargetX(int32_t currentTargetX, bool k1Down, bool k2Down, bool k1Held, bool k2Held)
{
    if ((k1Down and k2Held) or (k2Down and k1Held))
    {
        return kBoxCenterX;
    }
    if (k1Down)
    {
        return kBoxCenterX - kBoxStep;
    }
    if (k2Down)
    {
        return kBoxCenterX + kBoxStep;
    }
    return currentTargetX;
}

struct ButtonBoxState
{
    int32_t x_ = kBoxCenterX;
    int32_t targetX_ = kBoxCenterX;
    bool prevK1_ = false;
    bool prevK2_ = false;
};

void updateButtonState(ButtonBoxState &state)
{
    const bool k1 = keyPressed(Key1_GPIO_Port, Key1_Pin);
    const bool k2 = keyPressed(Key2_GPIO_Port, Key2_Pin);
    const bool k1Down = k1 and not state.prevK1_;
    const bool k2Down = k2 and not state.prevK2_;
    state.prevK1_ = k1;
    state.prevK2_ = k2;

    state.targetX_ = resolveTargetX(state.targetX_, k1Down, k2Down, k1, k2);
}

void stepBoxMotion(ButtonBoxState &state)
{
    const int32_t delta = state.targetX_ - state.x_;
    if (delta > kBoxSpeedPx)
    {
        state.x_ += kBoxSpeedPx;
    }
    else if (delta < -kBoxSpeedPx)
    {
        state.x_ -= kBoxSpeedPx;
    }
    else
    {
        state.x_ = state.targetX_;
    }
}

} // namespace

extern "C" void lcdTest(void)
{
    printTestStamp();

    const char banner[] = "lcdriv v20 working point @40MHz\r\n";
    uartWrite(banner, static_cast<uint16_t>(sizeof(banner) - 1));

    // 40 MHz: the one clock that produced a stable picture (v18 P4).
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    HAL_SPI_Init(&hspi1);

    lcd.emplace(&hspi1, kDcPin, kCsPins, kRstPins);

    ButtonBoxState state;
    composeFrame(state.x_);
    lcd->pushFrame(0, frameBuffer);

    uint32_t frames = 0;
    uint32_t lastReport = HAL_GetTick();

    while (1)
    {
        updateButtonState(state);
        stepBoxMotion(state);
        composeFrame(state.x_);
        lcd->pushFrame(0, frameBuffer);
        ++frames;

        if ((HAL_GetTick() - lastReport) >= 5000U)
        {
            lastReport = HAL_GetTick();
            uartText("frames=");
            uartNumber(frames);
            uartText(" box=");
            uartNumber(static_cast<uint32_t>(state.x_));
            uartText("\r\n");
        }
    }
}
