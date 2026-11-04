#include <SDL.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include "model.h"
#include "controller.h"
#include "runtimeAssets.h"

int main(int argc, char** argv) {
    try {
        SDL_SetMainReady();
        char* base = SDL_GetBasePath();
        if (!base) throw std::runtime_error("Cannot locate executable directory");
        std::filesystem::path assets = std::filesystem::u8path(base) / "assets";
        SDL_free(base);
        bool check = false, smoke = false;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--assets" && i+1 < argc) assets = argv[++i];
            else if (arg == "--check-assets") check = true;
            else if (arg == "--smoke-test") smoke = true;
            else if (arg == "--help") {
                std::cout << "soldier-skirmish [--assets DIRECTORY] [--check-assets | --smoke-test]\n";
                return 0;
            } else throw std::runtime_error("Unknown or incomplete argument: " + arg);
        }
        setAssetDirectory(assets);
        validateAssets();
        if (check) {
            std::cout << "Assets and map/base configuration are valid.\n";
            return 0;
        }
        // Detached simulation workers retain the model until process exit.
        // Managed shutdown remains a TODO in README.md.
        Model* model = new Model(assetPath("map.txt"), assetPath("base.txt"));
        Controller controller(model, smoke);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Soldier Skirmish: " << error.what() << '\n';
        return 1;
    }
}
