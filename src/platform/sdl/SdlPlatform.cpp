// src/platform/sdl/SdlPlatform.cpp — SDL 后端实现
#include "platform/sdl/SdlPlatform.hpp"

#include <SDL.h>

namespace ray
{

namespace
{
constexpr Uint32 kPixelFormat = SDL_PIXELFORMAT_RGB24;
} // namespace

SdlPlatform::SdlPlatform(int width, int height) : width_(width), height_(height)
{
    // 注意：SDL_Init 必须先于任何创建调用，因此资源不能在初始化列表中创建。
    SDL_Init(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("Raycaster Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width_, height_, SDL_WINDOW_RESIZABLE);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED); // NOLINT
    texture_ = SDL_CreateTexture(renderer_, kPixelFormat, SDL_TEXTUREACCESS_STREAMING, width_,
                                 height_); // NOLINT
}

SdlPlatform::~SdlPlatform()
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

uint32_t SdlPlatform::getTicks()
{
    return SDL_GetTicks();
}

void SdlPlatform::sampleInput(InputState &out)
{
    out.heldMask = 0;
    out.moveX = Fixed::fromInt(0);
    out.moveY = Fixed::fromInt(0);
    out.turnDelta = 0;
    out.quit = false;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            out.quit = true;
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    // SDL 键盘状态是 C API 返回的指针数组，必须按下标访问
    if (keys[SDL_SCANCODE_W] != 0)
    {
        out.moveY = out.moveY + Fixed::fromInt(1);
    }
    if (keys[SDL_SCANCODE_S] != 0)
    {
        out.moveY = out.moveY - Fixed::fromInt(1);
    }
    if (keys[SDL_SCANCODE_A] != 0)
    {
        out.moveX = out.moveX + Fixed::fromInt(1);
    }
    if (keys[SDL_SCANCODE_D] != 0)
    {
        out.moveX = out.moveX - Fixed::fromInt(1);
    }
    if (keys[SDL_SCANCODE_LEFT] != 0)
    {
        out.heldMask = out.heldMask | ACTION_TURN_LEFT;
    }
    if (keys[SDL_SCANCODE_RIGHT] != 0)
    {
        out.heldMask = out.heldMask | ACTION_TURN_RIGHT;
    }
    if (keys[SDL_SCANCODE_E] != 0)
    {
        out.heldMask = out.heldMask | ACTION_USE;
    }
    if (keys[SDL_SCANCODE_ESCAPE] != 0)
    {
        out.quit = true;
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // 读取鼠标相对移动
    int mouseDx = 0;
    int mouseDy = 0;
    SDL_GetRelativeMouseState(&mouseDx, &mouseDy);
    // SDL 相对鼠标：右移为正；FPS 惯例：鼠标右 → 视角右（需取反因为旋转矩阵正角=逆时针）
    out.turnDelta = -mouseDx;
}

void SdlPlatform::drawBuffer(const Color *pixels)
{
    SDL_UpdateTexture(texture_, nullptr, pixels, width_ * 3); // RGB24: 3 字节/像素
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

int SdlPlatform::screenWidth() const
{
    return width_;
}

int SdlPlatform::screenHeight() const
{
    return height_;
}

} // namespace ray
