#include "main.h"

#include <cstddef>

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

namespace
{

constexpr uint16_t kPanelWidth = 320;
constexpr uint16_t kPanelHeight = 240;
constexpr uint16_t kLineBytes = kPanelWidth * 2;

void uartWrite(const char *text, uint16_t length)
{
    HAL_UART_Transmit(&huart1, reinterpret_cast<const uint8_t *>(text), length, HAL_MAX_DELAY);
}

void printTestStamp()
{
    const char text[] = "Test Build: " __DATE__ " " __TIME__ "\r\n";
    uartWrite(text, static_cast<uint16_t>(sizeof(text) - 1));
}

void rawWriteParams(uint8_t command, const uint8_t *values, uint16_t length)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    uint8_t cmd = command;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    if (length > 0)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
        HAL_SPI_Transmit(&hspi1, const_cast<uint8_t *>(values), length, HAL_MAX_DELAY);
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void resetPanel()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_Delay(150);
}

uint8_t lineBuffer[kLineBytes];

void buildLine(uint16_t color, uint16_t pixelCount)
{
    const uint8_t high = static_cast<uint8_t>(color >> 8);
    const uint8_t low = static_cast<uint8_t>(color & 0xFF);
    for (uint16_t i = 0; i < pixelCount * 2; i += 2)
    {
        lineBuffer[i] = high;
        lineBuffer[i + 1] = low;
    }
}

void writePixels(uint16_t rows, uint16_t rowBytes)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    uint8_t cmd = 0x2C;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    for (uint16_t i = 0; i < rows; ++i)
    {
        HAL_SPI_Transmit(&hspi1, lineBuffer, rowBytes, HAL_MAX_DELAY);
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void fillPanel(uint16_t color)
{
    const uint8_t col[4] = {0x00, 0x00, static_cast<uint8_t>((kPanelWidth - 1) >> 8),
                            static_cast<uint8_t>(kPanelWidth - 1)};
    const uint8_t page[4] = {0x00, 0x00, static_cast<uint8_t>((kPanelHeight - 1) >> 8),
                             static_cast<uint8_t>(kPanelHeight - 1)};
    rawWriteParams(0x2A, col, 4);
    rawWriteParams(0x2B, page, 4);
    buildLine(color, kPanelWidth);
    writePixels(kPanelHeight, kPanelWidth * 2);
}

void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    const uint16_t xEnd = x + width - 1;
    const uint16_t yEnd = y + height - 1;
    const uint8_t col[4] = {static_cast<uint8_t>(x >> 8), static_cast<uint8_t>(x),
                            static_cast<uint8_t>(xEnd >> 8), static_cast<uint8_t>(xEnd)};
    const uint8_t page[4] = {static_cast<uint8_t>(y >> 8), static_cast<uint8_t>(y),
                             static_cast<uint8_t>(yEnd >> 8), static_cast<uint8_t>(yEnd)};
    rawWriteParams(0x2A, col, 4);
    rawWriteParams(0x2B, page, 4);
    buildLine(color, width);
    writePixels(height, width * 2);
}

void initPanelReset()
{
    rawWriteParams(0x01, nullptr, 0);
    HAL_Delay(120);
    rawWriteParams(0x11, nullptr, 0);
    HAL_Delay(120);
}

void initPanelConfig()
{
    const uint8_t colmod = 0x55;
    rawWriteParams(0x3A, &colmod, 1);
    const uint8_t madctl = 0x28;
    rawWriteParams(0x36, &madctl, 1);
    const uint8_t teon = 0x00;
    rawWriteParams(0x35, &teon, 1);
    const uint8_t pwctr1 = 0x23;
    rawWriteParams(0xC0, &pwctr1, 1);
    const uint8_t pwctr2 = 0x10;
    rawWriteParams(0xC1, &pwctr2, 1);
    const uint8_t vmctr1[2] = {0x3E, 0x28};
    rawWriteParams(0xC5, vmctr1, 2);
    const uint8_t vmctr2 = 0x86;
    rawWriteParams(0xC7, &vmctr2, 1);
    const uint8_t frmctr1[2] = {0x00, 0x18};
    rawWriteParams(0xB1, frmctr1, 2);
    const uint8_t disctrl[3] = {0x08, 0x82, 0x27};
    rawWriteParams(0xB6, disctrl, 3);
    const uint8_t enable3g = 0x00;
    rawWriteParams(0xF2, &enable3g, 1);
    const uint8_t gamset = 0x01;
    rawWriteParams(0x26, &gamset, 1);
    rawWriteParams(0x20, nullptr, 0);
}

void initPanel()
{
    initPanelReset();
    initPanelConfig();
    rawWriteParams(0x29, nullptr, 0);
    HAL_Delay(50);
}

struct StrokeRect
{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
};

constexpr StrokeRect kGlyphStrokes[] = {
    {80, 179, 160, 16},
    {130, 45, 16, 150},
    {130, 125, 84, 16},
};

template <std::size_t N>
void drawStrokes(const StrokeRect (&strokes)[N], uint16_t color)
{
    for (const StrokeRect &stroke : strokes)
    {
        fillRect(stroke.x, stroke.y, stroke.width, stroke.height, color);
    }
}

} // namespace

extern "C" void lcdTest(void)
{
    printTestStamp();
    resetPanel();
    initPanel();
    fillPanel(0x0000);
    drawStrokes(kGlyphStrokes, 0xF800);

    while (1)
    {
        HAL_Delay(1000);
    }
}
