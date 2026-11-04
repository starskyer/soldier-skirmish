#include "view.h"
#include "runtimeAssets.h"
#include <algorithm>
#include <stdexcept>

namespace skirmish {
namespace {
constexpr int Width = WINDOW_WIDTH, Height = WINDOW_HEIGHT, BaseBar = BASEBAR_HEIGHT,
              Menu = MENU_HEIGHT;
using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>;
void require(bool valid, const char *message) {
    if (!valid)
        throw std::runtime_error(std::string(message) + ": " + SDL_GetError());
}
} // namespace
SdlRuntime::SdlRuntime() {
    try {
        require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0, "SDL initialization failed");
        require((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0, "PNG initialization failed");
        require(TTF_Init() == 0, "Font initialization failed");
    } catch (...) {
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        throw;
    }
}
SdlRuntime::~SdlRuntime() {
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
View::View() {
    int w = Width, h = Height;
    SDL_Rect available{};
    if (SDL_GetDisplayUsableBounds(0, &available) == 0 && available.w > 0 && available.h > 0) {
        const double scale =
            std::min({1.0, static_cast<double>(std::max(240, available.w - 60)) / Width,
                      static_cast<double>(std::max(300, available.h - 80)) / Height});
        w = static_cast<int>(Width * scale);
        h = static_cast<int>(Height * scale);
    }
    window_.reset(SDL_CreateWindow("Soldier Skirmish", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, w, h,
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));
    require(static_cast<bool>(window_), "Window creation failed");
    renderer_.reset(SDL_CreateRenderer(window_.get(), -1,
                                       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if (!renderer_)
        renderer_.reset(SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_SOFTWARE));
    require(static_cast<bool>(renderer_), "Renderer creation failed");
    require(SDL_RenderSetLogicalSize(renderer_.get(), Width, Height) == 0,
            "Logical size setup failed");
    font_.reset(TTF_OpenFont(assetPath("font/NotoSans.ttf").c_str(), 28));
    require(static_cast<bool>(font_), "Font loading failed");
    const std::array<const char *, 7> names = {"Infantry1.png", "Infantry2.png", "Archer1.png",
                                               "Archer2.png",   "SiegeGun1.png", "SiegeGun2.png",
                                               "Base.png"};
    for (std::size_t i = 0; i < names.size(); ++i)
        textures_[i] = texture(assetPath(std::string("texture/") + names[i]));
}
View::Texture View::texture(const std::string &path) {
    Surface surface(IMG_Load(path.c_str()), SDL_FreeSurface);
    require(static_cast<bool>(surface), "Image loading failed");
    Texture result(SDL_CreateTextureFromSurface(renderer_.get(), surface.get()));
    require(static_cast<bool>(result), "Texture creation failed");
    return result;
}
void View::text(const std::string &value, int x, int y, bool centered) {
    Surface surface(TTF_RenderUTF8_Blended(font_.get(), value.c_str(), SDL_Color{15, 15, 15, 255}),
                    SDL_FreeSurface);
    require(static_cast<bool>(surface), "Text rendering failed");
    Texture label(SDL_CreateTextureFromSurface(renderer_.get(), surface.get()));
    require(static_cast<bool>(label), "Text texture creation failed");
    SDL_Rect rect{x - (centered ? surface->w / 2 : 0), y, surface->w, surface->h};
    require(SDL_RenderCopy(renderer_.get(), label.get(), nullptr, &rect) == 0, "Text copy failed");
}
int View::buttonAt(const SDL_MouseButtonEvent &event) const {
    if (event.button != SDL_BUTTON_LEFT)
        return -1;
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_.get(), event.x, event.y, &x, &y);
    if (y < Height - 3 * Menu / 4 || y > Height - Menu / 4)
        return -1;
    for (int i = 0; i < 3; ++i) {
        const int left = Width / 12 + i * Width / 3;
        if (x >= left && x <= left + Width / 6)
            return i;
    }
    return -1;
}
void View::capture(const std::filesystem::path &path) {
    int w = 0, h = 0;
    require(SDL_GetRendererOutputSize(renderer_.get(), &w, &h) == 0, "Capture size failed");
    Surface surface(SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32),
                    SDL_FreeSurface);
    require(static_cast<bool>(surface), "Capture allocation failed");
    require(SDL_RenderReadPixels(renderer_.get(), nullptr, surface->format->format, surface->pixels,
                                 surface->pitch) == 0,
            "Capture failed");
    require(IMG_SavePNG(surface.get(), path.u8string().c_str()) == 0, "PNG save failed");
}
void View::render(const Model &model, int vision, double alpha,
                  const std::filesystem::path &screenshot) {
    auto *renderer = renderer_.get();
    const auto &board = model.board();
    const auto &state = model.state();
    const float cw = static_cast<float>(Width) / board.columns(),
                ch = static_cast<float>(Height - BaseBar - Menu) / board.rows();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    for (int i = 0; i < board.size(); ++i) {
        const auto p = board.cell(i);
        const bool visible = model.vision().visible(vision, i);
        SDL_SetRenderDrawColor(renderer, visible ? (board.terrain(p) ? 35 : 255) : 145,
                               visible ? (board.terrain(p) ? 35 : 255) : 145,
                               visible ? (board.terrain(p) ? 35 : 255) : 145, 255);
        SDL_FRect tile{p.x * cw, BaseBar + p.y * ch, cw, ch};
        SDL_RenderFillRectF(renderer, &tile);
    }
    SDL_SetRenderDrawColor(renderer, 185, 185, 185, 255);
    for (int x = 0; x <= board.columns(); ++x)
        SDL_RenderDrawLine(renderer, static_cast<int>(x * cw), BaseBar, static_cast<int>(x * cw),
                           Height - Menu);
    for (int y = 0; y <= board.rows(); ++y)
        SDL_RenderDrawLine(renderer, 0, static_cast<int>(BaseBar + y * ch), Width,
                           static_cast<int>(BaseBar + y * ch));
    for (const auto &base : state.bases) {
        bool visible = false;
        for (Cell p : board.around(base.center, BaseRadius))
            visible = visible || model.vision().visible(vision, board.index(p));
        if (visible) {
            SDL_FRect rect{(base.center.x - BaseRadius) * cw + 1,
                           BaseBar + (base.center.y - BaseRadius) * ch + 1, BASE_SIZE * cw - 2,
                           BASE_SIZE * ch - 2};
            SDL_RenderCopyF(renderer, textures_[6].get(), nullptr, &rect);
        }
    }
    for (const auto &entry : state.units) {
        const auto &unit = entry.second;
        if (!model.vision().visible(vision, board.index(unit.position)))
            continue;
        double x = unit.position.x, y = unit.position.y;
        if (unit.motion.end > unit.motion.begin) {
            const double t =
                std::clamp((static_cast<double>(state.tick) + std::clamp(alpha, 0.0, 1.0) -
                            unit.motion.begin) /
                               static_cast<double>(unit.motion.end - unit.motion.begin),
                           0.0, 1.0);
            x = unit.motion.from.x + (unit.motion.to.x - unit.motion.from.x) * t;
            y = unit.motion.from.y + (unit.motion.to.y - unit.motion.from.y) * t;
        }
        SDL_FRect rect{static_cast<float>(x * cw + 1),
                       static_cast<float>(BaseBar + y * ch + ch * 0.1f + 1), std::max(1.0f, cw - 2),
                       std::max(1.0f, ch * 0.9f - 2)};
        const int image = static_cast<int>(unit.kind) * 2 + teamIndex(unit.team);
        SDL_RenderCopyF(renderer, textures_.at(image).get(), nullptr, &rect);
        SDL_SetRenderDrawColor(renderer, unit.team == Team::Red ? 220 : 30, 40,
                               unit.team == Team::Red ? 40 : 220, 255);
        SDL_FRect health{static_cast<float>(x * cw + 1), static_cast<float>(BaseBar + y * ch + 1),
                         static_cast<float>((cw - 2) * unit.hp / unit.stats.maxHP),
                         std::max(1.0f, ch * 0.1f)};
        SDL_RenderFillRectF(renderer, &health);
    }
    SDL_SetRenderDrawColor(renderer, 248, 248, 248, 255);
    SDL_Rect header{0, 0, Width, BaseBar};
    SDL_RenderFillRect(renderer, &header);
    for (int i = 0; i < 2; ++i) {
        SDL_SetRenderDrawColor(renderer, i ? 50 : 220, 50, i ? 220 : 50, 255);
        SDL_Rect bar{i * Width / 2, 0, (Width / 2) * state.bases[i].hp / state.bases[i].maxHP,
                     BaseBar};
        SDL_RenderFillRect(renderer, &bar);
    }
    text("Red: " + std::to_string(state.bases[0].hp), 15, 7);
    text("Blue: " + std::to_string(state.bases[1].hp), Width / 2 + 15, 7);
    SDL_SetRenderDrawColor(renderer, 248, 248, 248, 255);
    SDL_Rect menu{0, Height - Menu, Width, Menu};
    SDL_RenderFillRect(renderer, &menu);
    const std::array<std::string, 3> labels = {
        state.status == MatchStatus::Ready ? "Start" : "Started",
        state.status == MatchStatus::Paused ? "Resume" : "Pause",
        "Vision: " + std::to_string(vision)};
    for (int i = 0; i < 3; ++i) {
        SDL_Rect button{Width / 12 + i * Width / 3, Height - 3 * Menu / 4, Width / 6, Menu / 2};
        SDL_SetRenderDrawColor(renderer, 65, 65, 65, 255);
        SDL_RenderDrawRect(renderer, &button);
        text(labels[i], Width / 6 + i * Width / 3, Height - Menu / 2 - 18, true);
    }
    if (finished(state.status)) {
        SDL_SetRenderDrawColor(renderer, 250, 250, 250, 255);
        SDL_Rect panel{Width / 8, Height / 2 - 40, 3 * Width / 4, 85};
        SDL_RenderFillRect(renderer, &panel);
        text(state.status == MatchStatus::Draw     ? "Draw - both bases destroyed"
             : state.status == MatchStatus::RedWon ? "Red wins"
                                                   : "Blue wins",
             Width / 2, Height / 2 - 20, true);
    }
    if (!screenshot.empty())
        capture(screenshot);
    SDL_RenderPresent(renderer);
}
} // namespace skirmish
