#pragma once

#include "base/Pixel.hpp"
#include "base/Resolution.hpp"

#include <cstddef>

namespace onyx::base
{

template <typename PixelT, Resolution ResolutionT>
class FrameBufferT
{
  public:
    using Pixel = PixelT;

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

    [[nodiscard]] static constexpr std::size_t size() noexcept
    {
        return static_cast<std::size_t>(kInfo.width_) * static_cast<std::size_t>(kInfo.height_);
    }

    [[nodiscard]] PixelT &pixelAt(int x, int y) const noexcept
    {
        return pixels_[(y * stride()) + x];
    }

    [[nodiscard]] PixelT *data() const noexcept
    {
        return pixels_;
    }

    void setColor(int x, int y, Color color) const noexcept
    {
        pixelAt(x, y) = PixelT::fromColor(color);
    }

    void fillColor(int x0, int y0, int x1, int y1, Color color) const noexcept
    {
        if (not intersectsViewport(x0, y0, x1, y1))
        {
            return;
        }

        const int left = clampToRange(x0, 0, width() - 1);
        const int right = clampToRange(x1, 0, width() - 1);
        const int top = clampToRange(y0, 0, height() - 1);
        const int bottom = clampToRange(y1, 0, height() - 1);

        const PixelT encoded = PixelT::fromColor(color);

        for (int y = top; y <= bottom; ++y)
        {
            for (int x = left; x <= right; ++x)
            {
                pixelAt(x, y) = encoded;
            }
        }
    }

  private:
    [[nodiscard]] static constexpr bool intersectsViewport(int x0, int y0, int x1, int y1) noexcept
    {
        const bool overlaps = (x1 >= 0) and (y1 >= 0) and (x0 < width()) and (y0 < height());
        return overlaps and (x0 <= x1) and (y0 <= y1);
    }

    [[nodiscard]] static constexpr int clampToRange(int value, int low, int high) noexcept
    {
        if (value < low)
        {
            return low;
        }
        if (value > high)
        {
            return high;
        }
        return value;
    }

    static constexpr ResolutionInfo kInfo = kResolutionTable[static_cast<std::size_t>(ResolutionT)];

    PixelT *pixels_;
};

} // namespace onyx::base
