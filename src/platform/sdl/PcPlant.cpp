#include "platform/sdl/PcPlant.hpp"

#include "algos/Fixed.hpp"

#include <SDL.h>

namespace onyx::platform
{

constexpr Uint32 kPixelFormat = SDL_PIXELFORMAT_RGB24;
constexpr int kBytesPerPixel = static_cast<int>(sizeof(base::Rgb888));

PcPlant::PcPlant()
{
    openWindow();
}

PcPlant::~PcPlant()
{
    if (texture_ != nullptr)
    {
        SDL_DestroyTexture(texture_);
    }
    if (renderer_ != nullptr)
    {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_ != nullptr)
    {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

void PcPlant::openWindow()
{
    SDL_Init(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("Raycaster Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               kWidth, kHeight, SDL_WINDOW_RESIZABLE);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED); // NOLINT
    texture_ = SDL_CreateTexture(renderer_, kPixelFormat, SDL_TEXTUREACCESS_STREAMING, kWidth,
                                 kHeight); // NOLINT
}

void PcPlant::pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
            quitRequested_ = true;
        }
    }

    int mouseDx = 0;
    int mouseDy = 0;
    SDL_GetRelativeMouseState(&mouseDx, &mouseDy);
    pointerTurnDelta_ = -mouseDx;
}

void PcPlant::readMoveKeys(const Uint8 *keys, Input &input)
{
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (keys[SDL_SCANCODE_W] != 0)
    {
        input.moveY_ = input.moveY_ + algos::Fixed::fromInt(1);
    }
    if (keys[SDL_SCANCODE_S] != 0)
    {
        input.moveY_ = input.moveY_ - algos::Fixed::fromInt(1);
    }
    if (keys[SDL_SCANCODE_A] != 0)
    {
        input.moveX_ = input.moveX_ + algos::Fixed::fromInt(1);
    }
    if (keys[SDL_SCANCODE_D] != 0)
    {
        input.moveX_ = input.moveX_ - algos::Fixed::fromInt(1);
    }
    if (keys[SDL_SCANCODE_ESCAPE] != 0)
    {
        quitRequested_ = true;
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

int PcPlant::readKeyTurn(const Uint8 *keys) const
{
    int keyTurn = 0;
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (keys[SDL_SCANCODE_RIGHT] != 0)
    {
        keyTurn = keyTurn - 1;
    }
    if (keys[SDL_SCANCODE_LEFT] != 0)
    {
        keyTurn = keyTurn + 1;
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return keyTurn;
}

PcPlant::Input PcPlant::buildInput()
{
    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    Input input{};

    readMoveKeys(keys, input);

    const int32_t turnRaw = (readKeyTurn(keys) * contracts::doomlike::kKeyTurnStep.raw()) +
                            (pointerTurnDelta_ * contracts::doomlike::kPointerTurnScale);
    input.turn_ = algos::Fixed::fromRaw(turnRaw);
    return input;
}

PcPlant::Input PcPlant::sampleInputImpl()
{
    pollEvents();
    return buildInput();
}

void PcPlant::putSnapshotImpl(const Snapshot &snapshot)
{
    snapshots_[writeIndex_] = snapshot;
    readIndex_ = writeIndex_;
    writeIndex_ = 1 - writeIndex_;
    hasSnapshot_ = true;
}

const PcPlant::Snapshot *PcPlant::getSnapshotImpl() const noexcept
{
    return hasSnapshot_ ? &snapshots_[readIndex_] : nullptr;
}

base::Rgb888 *PcPlant::pixelBufferImpl() noexcept
{
    return pixels_;
}

void PcPlant::submitFrameImpl()
{
    SDL_UpdateTexture(texture_, nullptr, pixels_, kWidth * kBytesPerPixel);
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

uint32_t PcPlant::nowMs() const noexcept
{
    return SDL_GetTicks();
}

bool PcPlant::quitRequested() const noexcept
{
    return quitRequested_;
}

} // namespace onyx::platform
