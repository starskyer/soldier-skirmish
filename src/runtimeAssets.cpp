#include "runtimeAssets.h"
#include "chessboard.h"
#include <array>
#include <stdexcept>
namespace {
std::filesystem::path directory;
}
void setAssetDirectory(const std::filesystem::path &value) {
    directory = value;
}
std::string assetPath(const std::string &relative) {
    return (directory / relative).u8string();
}
void validateAssets() {
    const std::array<const char *, 10> files = {"map.txt",
                                                "base.txt",
                                                "font/NotoSans.ttf",
                                                "texture/Infantry1.png",
                                                "texture/Infantry2.png",
                                                "texture/Archer1.png",
                                                "texture/Archer2.png",
                                                "texture/SiegeGun1.png",
                                                "texture/SiegeGun2.png",
                                                "texture/Base.png"};
    for (const char *file : files)
        if (!std::filesystem::is_regular_file(directory / file) ||
            std::filesystem::file_size(directory / file) == 0)
            throw std::runtime_error("Missing or empty asset: " + assetPath(file));
    (void)skirmish::Chessboard::load(directory / "map.txt", directory / "base.txt");
}
