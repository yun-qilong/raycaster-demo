// src/platform/sdl/SdlPlatform.hpp — SDL 后端（PC：窗口/键盘/时钟）
#pragma once

#include "platform/api/Platform.hpp"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace ray
{

class SdlPlatform final : public Platform
{
  public:
    SdlPlatform(int width, int height);
    ~SdlPlatform() override;

    SdlPlatform(const SdlPlatform &) = delete;
    SdlPlatform &operator=(const SdlPlatform &) = delete;

    uint32_t getTicks() override;
    void sampleInput(InputState &out) override;
    void drawBuffer(const Color *pixels) override;
    [[nodiscard]] int screenWidth() const override;
    [[nodiscard]] int screenHeight() const override;

  private:
    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_Texture *texture_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    int mouseDeltaX_ = 0;
    int mouseDeltaY_ = 0;
};

} // namespace ray
