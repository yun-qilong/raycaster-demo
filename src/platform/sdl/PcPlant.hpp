#pragma once

#include "base/Pixel.hpp"
#include "base/PlantBase.hpp"
#include "contracts/doomlikeContract/Contract.hpp"

#include <cstddef>
#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace onyx::platform
{

class PcPlant : public base::PlantBase<PcPlant, contracts::doomlike::Contract, base::Rgb888,
                                       base::Resolution::k640x400>
{
  public:
    using Base = base::PlantBase<PcPlant, contracts::doomlike::Contract, base::Rgb888,
                                 base::Resolution::k640x400>;
    using FrameBuffer = Base::FrameBuffer;
    using Input = contracts::doomlike::Input;
    using Snapshot = contracts::doomlike::Snapshot;

    static constexpr int kWidth = FrameBuffer::width();
    static constexpr int kHeight = FrameBuffer::height();

    PcPlant();
    ~PcPlant();

    PcPlant(const PcPlant &) = delete;
    PcPlant &operator=(const PcPlant &) = delete;

    Input sampleInputImpl();
    void putSnapshotImpl(const Snapshot &snapshot);
    [[nodiscard]] const Snapshot *getSnapshotImpl() const noexcept;
    [[nodiscard]] base::Rgb888 *pixelBufferImpl() noexcept;
    void submitFrameImpl();

    [[nodiscard]] uint32_t nowMs() const noexcept;
    [[nodiscard]] bool quitRequested() const noexcept;

  private:
    void openWindow();
    void pollEvents();
    void readMoveKeys(const unsigned char *keys, Input &input);
    [[nodiscard]] int readKeyTurn(const unsigned char *keys) const;
    Input buildInput();

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_Texture *texture_ = nullptr;
    base::Rgb888 pixels_[static_cast<std::size_t>(kWidth) * static_cast<std::size_t>(kHeight)]{};
    Snapshot snapshots_[2]{};
    int writeIndex_ = 0;
    int readIndex_ = 1;
    bool hasSnapshot_ = false;
    bool quitRequested_ = false;
    int pointerTurnDelta_ = 0;
};

} // namespace onyx::platform
