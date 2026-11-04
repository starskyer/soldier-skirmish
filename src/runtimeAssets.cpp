#include "runtimeAssets.h"
#include <array>
#include <fstream>
#include <stdexcept>
#include <vector>
namespace { std::filesystem::path directory; }
void setAssetDirectory(const std::filesystem::path& value) { directory = value; }
std::string assetPath(const std::string& relative) { return (directory / relative).u8string(); }
void validateAssets() {
    const std::array<const char*, 10> files = {
        "map.txt", "base.txt", "font/NotoSans.ttf", "texture/Infantry1.png",
        "texture/Infantry2.png", "texture/Archer1.png", "texture/Archer2.png",
        "texture/SiegeGun1.png", "texture/SiegeGun2.png", "texture/Base.png"};
    for (const auto* file : files) {
        if (!std::filesystem::is_regular_file(directory / file) ||
            std::filesystem::file_size(directory / file) == 0)
            throw std::runtime_error("Missing or empty asset: " + assetPath(file));
    }
    std::ifstream map(directory / "map.txt");
    std::string line;
    std::getline(map, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    std::size_t used = 0;
    const int size = std::stoi(line, &used);
    if (used != line.size() || size <= 0 || size > 1000000)
        throw std::runtime_error("Invalid map size");
    std::vector<std::string> rows;
    while (std::getline(map, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.find_first_not_of("0123456789") != std::string::npos ||
            (!rows.empty() && line.size() != rows.front().size()))
            throw std::runtime_error("Map rows must be equal-width digits");
        rows.push_back(line);
    }
    if (rows.empty() || rows.size() * rows.front().size() != static_cast<std::size_t>(size))
        throw std::runtime_error("Map dimensions do not match declared cell count");
    const int width = static_cast<int>(rows.front().size());
    const int height = static_cast<int>(rows.size());
    if (width > 1080 || height > 1080)
        throw std::runtime_error("Map is too large for the renderer");
    std::ifstream bases(directory / "base.txt");
    std::array<int, 6> values{};
    for (auto& value : values)
        if (!(bases >> value)) throw std::runtime_error("base.txt requires six integers");
    std::string extra;
    if (bases >> extra) throw std::runtime_error("Unexpected data in base.txt");
    std::vector<bool> occupied(size, false);
    for (int i = 0; i < 2; ++i) {
        const int team = values[3*i], center = values[3*i+1], spawn = values[3*i+2];
        if (team != i+1 || center < 0 || center >= size || spawn < 0 || spawn >= size)
            throw std::runtime_error("Invalid base team or position");
        const int x = center % width, y = center / width;
        if (x < 1 || x+1 >= width || y < 1 || y+1 >= height)
            throw std::runtime_error("A 3x3 base extends beyond the map");
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                const int cell = center + dy*width + dx;
                if (occupied[cell] || rows[y+dy][x+dx] != '0')
                    throw std::runtime_error("Overlapping base or obstructed base footprint");
                occupied[cell] = true;
            }
        }
    }
    for (int i = 0; i < 2; ++i) {
        const int spawn = values[3*i+2];
        if (occupied[spawn] || rows[spawn/width][spawn%width] != '0')
            throw std::runtime_error("Soldier spawn is occupied or obstructed");
    }
}
