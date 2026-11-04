#include "controller.h"
#include <algorithm>
#include <chrono>
#include <iostream>

namespace skirmish {
void Controller::run(bool smokeTest, const std::filesystem::path &screenshot) {
    view_.render(model_, vision_);
    if (smokeTest) {
        model_.start();
        for (int i = 0; i < 120 && !finished(model_.state().status); ++i)
            model_.step();
        model_.togglePause();
        view_.render(model_, vision_, 0, screenshot);
        model_.stop();
        std::cout << "SDL startup, active/paused rendering and shutdown passed.\n";
        return;
    }
    using Clock = std::chrono::steady_clock;
    auto previous = Clock::now();
    std::optional<Clock::time_point> ended;
    FixedStepClock timer;
    SDL_Event event{};
    bool pending = false, quit = false;
    while (!quit) {
        const auto frameStart = Clock::now();
        bool resetClock = false;
        const auto handle = [&](const SDL_Event &e) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
                return;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                switch (view_.buttonAt(e.button)) {
                case 0:
                    if (model_.state().status == MatchStatus::Ready) {
                        model_.start();
                        timer.reset();
                        resetClock = true;
                    }
                    break;
                case 1:
                    if (model_.state().status == MatchStatus::Running ||
                        model_.state().status == MatchStatus::Paused) {
                        model_.togglePause();
                        resetClock = true;
                    }
                    break;
                case 2:
                    vision_ = (vision_ + 1) % 3;
                    break;
                default:
                    break;
                }
            }
        };
        if (pending) {
            handle(event);
            pending = false;
        }
        while (SDL_PollEvent(&event))
            handle(event);
        if (quit)
            break;
        const auto now = Clock::now();
        if (resetClock)
            previous = now;
        const double elapsed = std::chrono::duration<double>(now - previous).count();
        previous = now;
        const int steps = timer.consume(elapsed, model_.state().status == MatchStatus::Running);
        for (int i = 0; i < steps; ++i)
            model_.step();
        view_.render(model_, vision_, timer.alpha());
        if (finished(model_.state().status)) {
            if (!ended)
                ended = Clock::now();
            if (Clock::now() - *ended >= std::chrono::seconds(3))
                quit = true;
        }
        const auto spent =
            std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - frameStart)
                .count();
        if (!quit)
            pending = SDL_WaitEventTimeout(
                          &event, static_cast<int>(std::max<std::int64_t>(1, 16 - spent))) == 1;
    }
    model_.stop();
}
} // namespace skirmish
