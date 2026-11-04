#pragma once
#include "model.h"
#include <filesystem>

namespace skirmish {
struct AppOptions {
    std::filesystem::path assets, screenshot;
    SimulationOptions simulation;
    Tick ticks = 18000;
    bool help = false, check = false, smoke = false, headless = false, requireFinished = false;
};
AppOptions parseOptions(int argc, char **argv, std::filesystem::path defaultAssets);
void printHelp();
int runHeadless(Model &model, Tick limit, bool requireFinished);
} // namespace skirmish
