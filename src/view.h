#pragma once
#include "model.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <array>
#include <filesystem>
#include <memory>
#include <string>

namespace skirmish {
class SdlRuntime {
  public:
    SdlRuntime();
    ~SdlRuntime();
    SdlRuntime(const SdlRuntime &) = delete;
    SdlRuntime &operator=(const SdlRuntime &) = delete;
};
struct WindowDelete {
    void operator()(SDL_Window *p) const { SDL_DestroyWindow(p); }
};
struct RendererDelete {
    void operator()(SDL_Renderer *p) const { SDL_DestroyRenderer(p); }
};
struct TextureDelete {
    void operator()(SDL_Texture *p) const { SDL_DestroyTexture(p); }
};
struct FontDelete {
    void operator()(TTF_Font *p) const { TTF_CloseFont(p); }
};
class View {
  public:
    View();
    void render(const Model &model, int vision, double alpha = 0,
                const std::filesystem::path &screenshot = {});
    int buttonAt(const SDL_MouseButtonEvent &event) const;

  private:
    using Texture = std::unique_ptr<SDL_Texture, TextureDelete>;
    SdlRuntime runtime_;
    std::unique_ptr<SDL_Window, WindowDelete> window_;
    std::unique_ptr<SDL_Renderer, RendererDelete> renderer_;
    std::unique_ptr<TTF_Font, FontDelete> font_;
    std::array<Texture, 7> textures_;
    Texture texture(const std::string &path);
    void text(const std::string &value, int x, int y, bool centered = false);
    void capture(const std::filesystem::path &path);
};
} // namespace skirmish
