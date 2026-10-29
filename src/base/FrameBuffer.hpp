#pragma once

#include "base/Resolution.hpp"

#include <cstddef>

namespace onyx::base
{

template <typename PixelT, Resolution ResolutionT>
class FrameBufferT
{
  public:
    explicit constexpr FrameBufferT(PixelT *pixels) noexcept : pixels_(pixels) {}

    [[nodiscard]] static constexpr int width() noexcept
    {
        return kInfo.width_;
    }

    [[nodiscard]] static constexpr int height() noexcept
    {
        return kInfo.height_;
    }

    [[nodiscard]] static constexpr int stride() noexcept
    {
        return kInfo.width_;
    }

    [[nodiscard]] PixelT &pixelAt(int x, int y) const noexcept
    {
        return pixels_[y * stride() + x];
    }

    void fill(PixelT color) const noexcept
    {
        for (int y = 0; y < height(); ++y)
        {
            PixelT *row = pixels_ + y * stride();
            for (int x = 0; x < width(); ++x)
            {
                row[x] = color;
            }
        }
    }

  private:
    static constexpr ResolutionInfo kInfo = kResolutionTable[static_cast<std::size_t>(ResolutionT)];

    PixelT *pixels_;
};

} // namespace onyx::base
