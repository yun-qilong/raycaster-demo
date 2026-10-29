#pragma once

#include <array>
#include <cstdint>

namespace onyx::base
{

enum class Resolution : uint8_t
{
    k320x240 = 0,
    k640x400,
    k800x600,
    k1024x768,
    k720p,
    k1080p,
    k2k,
    k4k,
};

struct ResolutionInfo
{
    int width_;
    int height_;
};

inline constexpr std::array<ResolutionInfo, 8> kResolutionTable{
    ResolutionInfo{320, 240},   ResolutionInfo{640, 400},   ResolutionInfo{800, 600},
    ResolutionInfo{1024, 768},  ResolutionInfo{1280, 720},  ResolutionInfo{1920, 1080},
    ResolutionInfo{2560, 1440}, ResolutionInfo{3840, 2160},
};

} // namespace onyx::base
