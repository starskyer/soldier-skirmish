#include "appOptions.h"
#include <iostream>

int main(int argc, char **argv) {
    try {
        const auto assets =
            std::filesystem::absolute(std::filesystem::u8path(argv[0])).parent_path() / "assets";
        auto options = skirmish::parseOptions(argc, argv, assets);
        if (options.help) {
            skirmish::printHelp();
            return 0;
        }
        if (options.smoke || options.check || !options.screenshot.empty())
            throw std::invalid_argument("Use the graphical executable for SDL checks");
        skirmish::Model model(
            skirmish::Chessboard::load(options.assets / "map.txt", options.assets / "base.txt"),
            options.simulation);
        return skirmish::runHeadless(model, options.ticks, options.requireFinished);
    } catch (const std::exception &error) {
        std::cerr << "Simulation: " << error.what() << '\n';
        return 1;
    }
}
