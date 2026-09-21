#include "main.h"

#include <lcdriv.hpp>

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
constexpr uint16_t kRed = 0xF800;

using Lcd = LcdDriver<BusType::SPI, ControllerType::ILI9341, kPanelWidth, kPanelHeight, 1, true>;

std::optional<Lcd> lcd;

const GpioPin kDcPin{GPIOB, GPIO_PIN_0};
const GpioPin kCsPins[1] = {{GPIOA, GPIO_PIN_4}};
const GpioPin kRstPins[1] = {{GPIOB, GPIO_PIN_1}};

constexpr int kNumBuffers = 2;
__attribute__((section(".lcdFrameBuffer"), aligned(32)))
uint8_t frameBuffer[kNumBuffers][Lcd::kBytes];

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

void paintRect(uint8_t *buf, int32_t x, int32_t y, int32_t size, uint16_t color)
{
    const uint8_t high = static_cast<uint8_t>(color >> 8);
    const uint8_t low = static_cast<uint8_t>(color & 0xFF);
    for (int32_t row = 0; row < size; ++row)
    {
        uint8_t *pixel = &buf[((y + row) * kPanelWidth + x) * 2];
        for (int32_t i = 0; i < size; ++i)
        {
            pixel[i * 2] = high;
            pixel[i * 2 + 1] = low;
        }
    }
}

void composeFrame(uint8_t *buf, int32_t boxX)
{
    memset(buf, 0xFF, Lcd::kBytes);
    paintRect(buf, kGreenBoxX, kGreenBoxY, kBoxSize, kGreenBoxColor);
    paintRect(buf, boxX, kBoxY, kBoxSize, kBoxColor);
}

// Bit-banged recovery path, verified in v31/v33: a slow, fully controllable
// init + fill that pulls the panel out of states the 40 MHz path cannot clear.

void bbByte(uint8_t value)
{
    for (int bit = 7; bit >= 0; --bit)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, ((value >> bit) & 1U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        for (volatile int d = 0; d < 12; ++d)
        {
        }
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        for (volatile int d = 0; d < 12; ++d)
        {
        }
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }
}

void bbPinsAsGpio()
{
    GPIO_InitTypeDef init = {0};
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    init.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOA, &init);
    init.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOA, &init);
    init.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOA, &init);
    init.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOB, &init);
    init.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &init);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

void bbCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    bbByte(cmd);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void bbCommandParams(uint8_t cmd, const uint8_t *params, uint16_t count)
{
    bbCommand(cmd);
    if (count == 0)
    {
        return;
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    for (uint16_t i = 0; i < count; ++i)
    {
        bbByte(params[i]);
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void bbInitPanel()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_Delay(150);

    bbCommand(0x01);
    HAL_Delay(120);
    bbCommand(0x11);
    HAL_Delay(120);

    const uint8_t colmod = 0x55;
    bbCommandParams(0x3A, &colmod, 1);
    const uint8_t madctl = 0x28;
    bbCommandParams(0x36, &madctl, 1);
    const uint8_t teon = 0x00;
    bbCommandParams(0x35, &teon, 1);
    const uint8_t pwctr1 = 0x23;
    bbCommandParams(0xC0, &pwctr1, 1);
    const uint8_t pwctr2 = 0x10;
    bbCommandParams(0xC1, &pwctr2, 1);
    const uint8_t vmctr1[2] = {0x3E, 0x28};
    bbCommandParams(0xC5, vmctr1, 2);
    const uint8_t vmctr2 = 0x86;
    bbCommandParams(0xC7, &vmctr2, 1);
    const uint8_t frmctr1[2] = {0x00, 0x18};
    bbCommandParams(0xB1, frmctr1, 2);
    const uint8_t disctrl[3] = {0x08, 0x82, 0x27};
    bbCommandParams(0xB6, disctrl, 3);
    const uint8_t enable3g = 0x00;
    bbCommandParams(0xF2, &enable3g, 1);
    const uint8_t gamset = 0x01;
    bbCommandParams(0x26, &gamset, 1);
    bbCommand(0x20);
    bbCommand(0x29);
    HAL_Delay(50);
}

void bbFillScreen(uint16_t color)
{
    const uint8_t col[4] = {0x00, 0x00, 0x01, 0x3F};
    const uint8_t page[4] = {0x00, 0x00, 0x00, 0xEF};
    bbCommandParams(0x2A, col, 4);
    bbCommandParams(0x2B, page, 4);

    const uint8_t hi = static_cast<uint8_t>(color >> 8);
    const uint8_t lo = static_cast<uint8_t>(color & 0xFF);

    bbCommand(0x2C);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    for (int32_t i = 0; i < kPanelWidth * kPanelHeight; ++i)
    {
        bbByte(hi);
        bbByte(lo);
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
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

    const char banner[] = "lcdriv DMA double-buffer keys + bitbang @40MHz\r\n";
    uartWrite(banner, static_cast<uint16_t>(sizeof(banner) - 1));

    // S1: bit-banged clear, the recovery path verified in v31/v33.
    uartText("S1 bitbang clear RED\r\n");
    bbPinsAsGpio();
    bbInitPanel();
    bbFillScreen(kRed);
    HAL_Delay(3000);

    // S2: pins back to SPI1 at 40 MHz, library init, one static frame.
    HAL_SPI_DeInit(&hspi1);
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_04DATA;
    HAL_SPI_Init(&hspi1);
    uartText("S2 library init + first frame\r\n");
    lcd.emplace(&hspi1, kDcPin, kCsPins, kRstPins);

    composeFrame(frameBuffer[0], kBoxCenterX);
    if (lcd->pushFrame(0, frameBuffer[0]))
    {
        uartText("first frame sent\r\n");
    }
    else
    {
        uartText("first frame REJECTED\r\n");
    }
    HAL_Delay(3000);

    // S3: full speed, Key1/Key2 move the red block (8 px per frame).
    uartText("S3 keys drive the red block\r\n");
    ButtonBoxState state;
    int render = 1;
    uint32_t frames = 1;
    uint32_t attempts = 0;
    uint32_t lastReport = HAL_GetTick();

    while (1)
    {
        updateButtonState(state);
        composeFrame(frameBuffer[render], state.x_);
        ++attempts;
        if (lcd->pushFrame(0, frameBuffer[render]))
        {
            render ^= 1;
            ++frames;
            stepBoxMotion(state);
        }

        if ((HAL_GetTick() - lastReport) >= 5000U)
        {
            lastReport = HAL_GetTick();
            uartText("frames=");
            uartNumber(frames);
            uartText(" attempts=");
            uartNumber(attempts);
            uartText(" spi=");
            uartNumber(hspi1.State);
            uartText("\r\n");
            attempts = 0;
        }
    }
}
