// pc/tests/TestAppPixelFormat.cpp — 像素编码由组装决定，应用层与编码无关
//
// ① RGB565 组装直接渲染的结果，必须等于 RGB888 组装渲染后按 565 截断的结果：
//    把 "MCU 帧 == pack(PC 基准)" 的对照口径在 PC 上先行锁定。
// ② RGB888 组装在固定输入脚本下的帧哈希，必须等于 FT0003-A 重构前实现的帧哈希：
//    守护 "PC 画面和现在完全一样"。期望值取自重构前（18f8f43^）的实现，
//    画面被有意改变时需同步更新。
#include "app/backroomLikeDemo/Game.hpp"
#include "base/Pixel.hpp"
#include "base/PlantBase.hpp"
#include "contracts/doomlikeContract/Contract.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

namespace
{

using onyx::base::Color;
using onyx::base::Resolution;
using onyx::base::Rgb565;
using onyx::base::Rgb888;
using onyx::contracts::doomlike::Input;

// 无平台依赖的桩 plant:缓冲与信箱都在本地，渲染走真实应用代码。
template <typename PixelT, Resolution ResolutionT>
class StubPlant
    : public onyx::base::PlantBase<StubPlant<PixelT, ResolutionT>,
                                   onyx::contracts::doomlike::Contract, PixelT, ResolutionT>
{
  public:
    using Base =
        onyx::base::PlantBase<StubPlant, onyx::contracts::doomlike::Contract, PixelT, ResolutionT>;
    using FrameBuffer = typename Base::FrameBuffer;
    using Snapshot = onyx::contracts::doomlike::Snapshot;
    using Pixels = std::array<PixelT, FrameBuffer::size()>;

    void setPending(const Input &input)
    {
        pending_ = input;
    }

    Input sampleInputImpl()
    {
        return pending_;
    }

    void putSnapshotImpl(const Snapshot &snapshot)
    {
        snapshot_ = snapshot;
        hasSnapshot_ = true;
    }

    [[nodiscard]] const Snapshot *getSnapshotImpl() const noexcept
    {
        return hasSnapshot_ ? &snapshot_ : nullptr;
    }

    [[nodiscard]] PixelT *pixelBufferImpl() noexcept
    {
        return pixels_.data();
    }

    void submitFrameImpl() {}

    [[nodiscard]] const Pixels &pixels() const noexcept
    {
        return pixels_;
    }

  private:
    Pixels pixels_{};
    Snapshot snapshot_{};
    Input pending_{};
    bool hasSnapshot_ = false;
};

// 固定输入脚本（确定性 LCG）：覆盖前进/后退/左右平移、按键转向与鼠标转向。
class InputScript
{
  public:
    [[nodiscard]] Input next()
    {
        const int32_t move = nextRaw();
        Input input;
        input.moveX_ = onyx::algos::Fixed::fromInt((move % 3) - 1);
        input.moveY_ = onyx::algos::Fixed::fromInt(((move / 3) % 3) - 1);

        const int32_t turn = nextRaw();
        int keyTurn = 0;
        if ((turn & 1) != 0)
        {
            keyTurn += 1;
        }
        if ((turn & 2) != 0)
        {
            keyTurn -= 1;
        }
        const int32_t pointerTurnDelta = ((turn >> 2) % 21) - 10;
        input.turn_ = onyx::algos::Fixed::fromRaw(
            (keyTurn * onyx::contracts::doomlike::kKeyTurnStep.raw()) +
            (pointerTurnDelta * onyx::contracts::doomlike::kPointerTurnScale));
        return input;
    }

  private:
    [[nodiscard]] int32_t nextRaw()
    {
        seed_ = static_cast<int32_t>(static_cast<uint32_t>(seed_) * 1103515245U + 12345U);
        return (seed_ >> 16) & 0x7fff;
    }

    int32_t seed_ = 12345;
};

template <typename PixelsT>
[[nodiscard]] uint64_t fnv1a(const PixelsT &pixels) noexcept
{
    uint64_t hash = 1469598103934665603ULL;
    const auto *bytes = reinterpret_cast<const uint8_t *>(pixels.data());

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (std::size_t i = 0; i < pixels.size() * sizeof(typename PixelsT::value_type); ++i)
    {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    return hash;
}

// RGB565 直写与 "RGB888 渲染后按 565 截断" 的不一致像素数。
template <typename Pixels565T, typename Pixels888T>
[[nodiscard]] std::size_t countTruncationMismatches(const Pixels565T &pixels565,
                                                    const Pixels888T &pixels888)
{
    std::size_t mismatches = 0;

    for (std::size_t i = 0; i < pixels565.size(); ++i)
    {
        const Rgb888 source = pixels888.at(i);
        const Rgb565 expected = Rgb565::fromColor(Color{source.r_, source.g_, source.b_});
        if (expected.bits_ != pixels565.at(i).bits_)
        {
            ++mismatches;
        }
    }

    return mismatches;
}

constexpr int kRgb565Frames = 30;

using Plant565 = StubPlant<Rgb565, Resolution::k320x240>;
using Plant888 = StubPlant<Rgb888, Resolution::k320x240>;
using Game565 = onyx::app::backroomLikeDemo::Game<Plant565>;
using Game888 = onyx::app::backroomLikeDemo::Game<Plant888>;

TEST(AppPixelFormatTest, Rgb565AssemblyMatchesTruncatedRgb888)
{
    static Plant565 plant565;
    static Plant888 plant888;
    static Game565 game565(plant565);
    static Game888 game888(plant888);

    InputScript script;

    for (int frame = 0; frame < kRgb565Frames; ++frame)
    {
        const Input input = script.next();

        plant565.setPending(input);
        game565.stepLogic();
        game565.presentFrame();

        plant888.setPending(input);
        game888.stepLogic();
        game888.presentFrame();

        EXPECT_EQ(countTruncationMismatches(plant565.pixels(), plant888.pixels()), std::size_t{0})
            << "frame " << frame;
    }
}

struct GoldenFrame
{
    std::size_t frame_;
    uint64_t hash_;
};

// RGB888 @640x400 的帧哈希（取自 FT0003-A 重构前的实现）。
constexpr std::array<GoldenFrame, 5> kGoldenFrameHashes{{
    {0, 0xce127a95a2316717ULL},
    {1, 0x4d0e1c1928123e8aULL},
    {37, 0x92bc84ad138a39f4ULL},
    {149, 0x01d2f4fbe1274790ULL},
    {299, 0x0edd0c8cfc56d2d2ULL},
}};

constexpr std::size_t kGoldenScriptFrames = 300;

using GoldenPlant = StubPlant<Rgb888, Resolution::k640x400>;
using GoldenGame = onyx::app::backroomLikeDemo::Game<GoldenPlant>;

TEST(AppPixelFormatTest, Rgb888GoldenMatchesPreRefactorBaseline)
{
    static GoldenPlant plant;
    static GoldenGame game(plant);

    InputScript script;
    std::size_t checked = 0;

    for (std::size_t frame = 0; frame < kGoldenScriptFrames; ++frame)
    {
        plant.setPending(script.next());
        game.stepLogic();
        game.presentFrame();

        const bool isGoldenFrame = (checked < kGoldenFrameHashes.size()) and
                                   (frame == kGoldenFrameHashes.at(checked).frame_);
        if (isGoldenFrame)
        {
            EXPECT_EQ(fnv1a(plant.pixels()), kGoldenFrameHashes.at(checked).hash_)
                << "frame " << frame;
            ++checked;
        }
    }

    EXPECT_EQ(checked, kGoldenFrameHashes.size());
}

} // namespace
