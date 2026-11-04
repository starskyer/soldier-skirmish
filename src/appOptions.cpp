#include "appOptions.h"
#include <iostream>
#include <limits>

namespace skirmish {
namespace {
std::uint64_t number(const std::string &value, std::uint64_t maximum) {
    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument("Expected an unsigned integer");
    const auto n = std::stoull(value);
    if (n > maximum)
        throw std::invalid_argument("Numeric argument exceeds limit");
    return n;
}
} // namespace
AppOptions parseOptions(int argc, char **argv, std::filesystem::path assets) {
    AppOptions options;
    options.assets = std::move(assets);
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const auto value = [&]() {
            if (i + 1 >= argc)
                throw std::invalid_argument("Missing value for " + arg);
            return std::string(argv[++i]);
        };
        if (arg == "--assets")
            options.assets = std::filesystem::u8path(value());
        else if (arg == "--seed")
            options.simulation.seed = static_cast<std::uint32_t>(
                number(value(), std::numeric_limits<std::uint32_t>::max()));
        else if (arg == "--ticks")
            options.ticks = number(value(), 10000000);
        else if (arg == "--screenshot")
            options.screenshot = std::filesystem::u8path(value());
        else if (arg == "--check-assets")
            options.check = true;
        else if (arg == "--smoke-test")
            options.smoke = true;
        else if (arg == "--headless")
            options.headless = true;
        else if (arg == "--require-finished")
            options.requireFinished = true;
        else if (arg == "--help")
            options.help = true;
        else
            throw std::invalid_argument("Unknown argument: " + arg);
    }
    if (static_cast<int>(options.check) + options.smoke + options.headless > 1)
        throw std::invalid_argument("Choose one execution mode");
    if (!options.screenshot.empty() && !options.smoke)
        throw std::invalid_argument("Screenshots require --smoke-test");
    return options;
}
void printHelp() {
    std::cout << "soldier-skirmish [--assets DIR] [--seed N] [--check-assets | --smoke-test | "
                 "--headless]\n"
              << "  --ticks N --require-finished   bounded headless simulation\n"
              << "  --screenshot FILE.png         save the smoke-test frame\n"
              << "soldier-skirmish-sim uses headless simulation without SDL.\n";
}
int runHeadless(Model &model, Tick limit, bool requireFinished) {
    model.start();
    for (Tick i = 0; i < limit && !finished(model.state().status); ++i)
        model.step();
    const auto &state = model.state();
    std::cout << "status=" << statusName(state.status) << " tick=" << state.tick
              << " units=" << state.units.size() << " red_hp=" << state.bases[0].hp
              << " blue_hp=" << state.bases[1].hp << " fingerprint=" << model.fingerprint() << '\n';
    return requireFinished && !finished(state.status) ? 2 : 0;
}
} // namespace skirmish
