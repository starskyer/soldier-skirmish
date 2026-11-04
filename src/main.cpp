#include "appOptions.h"
#include "controller.h"
#include "runtimeAssets.h"
#include <iostream>

int main(int argc, char **argv) {
    try {
        SDL_SetMainReady();
        std::unique_ptr<char, decltype(&SDL_free)> base(SDL_GetBasePath(), SDL_free);
        if (!base)
            throw std::runtime_error("Cannot locate executable directory");
        const auto options =
            skirmish::parseOptions(argc, argv, std::filesystem::u8path(base.get()) / "assets");
        if (options.help) {
            skirmish::printHelp();
            return 0;
        }
        if (options.headless) {
            skirmish::Model model(
                skirmish::Chessboard::load(options.assets / "map.txt", options.assets / "base.txt"),
                options.simulation);
            return skirmish::runHeadless(model, options.ticks, options.requireFinished);
        }
        setAssetDirectory(options.assets);
        validateAssets();
        if (options.check) {
            std::cout << "Assets and map/base configuration are valid.\n";
            return 0;
        }
        skirmish::Model model(
            skirmish::Chessboard::load(options.assets / "map.txt", options.assets / "base.txt"),
            options.simulation);
        skirmish::View view;
        skirmish::Controller controller(model, view);
        controller.run(options.smoke, options.screenshot);
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Soldier Skirmish: " << error.what() << '\n';
        return 1;
    }
}
