// src/platform/sdl/SdlPlatform.hpp — SDL 后端（PC：窗口/键盘/时钟）
#pragma once

#include "platform/api/Platform.hpp"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace ray
{

class SdlPlatform final : public Platform<SdlPlatform>
{
  public:
    SdlPlatform(int width, int height);
    ~SdlPlatform();

    SdlPlatform(const SdlPlatform &) = delete;
    SdlPlatform &operator=(const SdlPlatform &) = delete;

    uint32_t getTicksImpl();
    void sampleInputImpl(InputState &out);
    void drawBufferImpl(const Color *pixels);
    [[nodiscard]] int screenWidthImpl() const;
    [[nodiscard]] int screenHeightImpl() const;

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
