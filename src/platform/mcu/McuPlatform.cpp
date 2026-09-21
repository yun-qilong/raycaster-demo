#include "platform/mcu/McuPlatform.hpp"

namespace ray
{

namespace
{

constexpr uint16_t toRgb565(const Color &color)
{
    return static_cast<uint16_t>(((color.r_ & 0xF8U) << 8) | ((color.g_ & 0xFCU) << 3) |
                                 (color.b_ >> 3));
}

constexpr int kPixelCount = McuPlatform::kWidth * McuPlatform::kHeight;

} // namespace

McuPlatform::McuPlatform(SPI_HandleTypeDef *spi, GpioPin dc, GpioPin cs, GpioPin rst,
                         DualFrameBuffer &frameBuffer)
    : csPins_{cs}, rstPins_{rst}, frameBuffer_(frameBuffer)
{
    configureSpi(spi);
    lcd_.emplace(spi, dc, csPins_, rstPins_);
}

void McuPlatform::configureSpi(SPI_HandleTypeDef *spi)
{
    HAL_SPI_DeInit(spi);
    spi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    spi->Init.FifoThreshold = SPI_FIFO_THRESHOLD_04DATA;
    HAL_SPI_Init(spi);
}

uint32_t McuPlatform::getTicksImpl()
{
    return HAL_GetTick();
}

void McuPlatform::sampleInputImpl(InputState &out)
{
    out.moveX = Fixed::fromInt(0);
    out.moveY = Fixed::fromInt(0);
    out.turnDelta = 0;
    out.heldMask = 0;
    out.quit = false;
}

void McuPlatform::drawBufferImpl(const Color *pixels)
{
    convertFrame(pixels, frameBuffer_[writeIndex_]);
    if (lcd_->pushFrame(0, frameBuffer_[writeIndex_]))
    {
        writeIndex_ ^= 1;
    }
}

void McuPlatform::convertFrame(const Color *pixels, uint8_t *out)
{
    const Color *pixelIn = pixels;
    uint8_t *pixelOut = out;
    for (int i = 0; i < kPixelCount; ++i)
    {
        const uint16_t rgb565 = toRgb565(*pixelIn);
        pixelOut[0] = static_cast<uint8_t>(rgb565 >> 8);
        pixelOut[1] = static_cast<uint8_t>(rgb565 & 0xFFU);
        ++pixelIn;
        pixelOut += 2;
    }
}

int McuPlatform::screenWidthImpl() const
{
    return kWidth;
}

int McuPlatform::screenHeightImpl() const
{
    return kHeight;
}

} // namespace ray
