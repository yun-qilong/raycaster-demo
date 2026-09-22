// src/platform/mcu/McuPlatform.hpp — MCU 后端（STM32H743 + ILI9341 SPI 屏）
#pragma once

#include "platform/api/Platform.hpp"

#include "main.h"

#include <lcdriv.hpp>

#include <cstdint>
#include <optional>

namespace ray
{

class McuPlatform final : public Platform<McuPlatform>
{
  public:
    static constexpr int kWidth = 320;
    static constexpr int kHeight = 240;

    using Lcd = LcdDriver<BusType::SPI, ControllerType::ILI9341, kWidth, kHeight, 1, true>;
    using DualFrameBuffer = uint8_t[2][Lcd::kBytes];

    McuPlatform(SPI_HandleTypeDef *spi, GpioPin dc, GpioPin cs, GpioPin rst,
                DualFrameBuffer &frameBuffer);

    McuPlatform(const McuPlatform &) = delete;
    McuPlatform &operator=(const McuPlatform &) = delete;
    McuPlatform(McuPlatform &&) = delete;
    McuPlatform &operator=(McuPlatform &&) = delete;

    uint32_t getTicksImpl();
    void sampleInputImpl(InputState &out);
    void drawBufferImpl(const Color *pixels);
    [[nodiscard]] int screenWidthImpl() const;
    [[nodiscard]] int screenHeightImpl() const;
    [[nodiscard]] uint32_t renderedFrames() const;
    [[nodiscard]] uint32_t pushedFrames() const;

  private:
    void configureSpi(SPI_HandleTypeDef *spi);
    void convertFrame(const Color *pixels, uint8_t *out);

    GpioPin csPins_[1];
    GpioPin rstPins_[1];
    std::optional<Lcd> lcd_;
    DualFrameBuffer &frameBuffer_;
    int writeIndex_ = 0;
    uint32_t renderedFrames_ = 0;
    uint32_t pushedFrames_ = 0;
};

} // namespace ray
