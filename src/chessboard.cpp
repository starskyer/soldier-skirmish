#include "chessboard.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <string>

namespace skirmish {
int squareDistance(Cell a, Cell b) {
    return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
}
bool Chessboard::contains(Cell p) const {
    return p.x >= 0 && p.y >= 0 && p.x < columns_ && p.y < rows_;
}
int Chessboard::index(Cell p) const {
    if (!contains(p))
        throw std::out_of_range("Cell outside battlefield");
    return p.y * columns_ + p.x;
}
Cell Chessboard::cell(int i) const {
    if (i < 0 || i >= size())
        throw std::out_of_range("Invalid cell index");
    return {i % columns_, i / columns_};
}
int Chessboard::terrain(Cell p) const {
    return terrain_.at(index(p));
}
int Chessboard::baseAt(Cell p) const {
    return baseIndex_.at(index(p));
}
bool Chessboard::walkable(Cell p) const {
    return contains(p) && terrain(p) == 0 && baseAt(p) == -1;
}
int Chessboard::distanceToBase(Cell p, Team team) const {
    const auto center = bases_.at(teamIndex(team)).center;
    return std::max(
        {0, std::abs(p.x - center.x) - BaseRadius, std::abs(p.y - center.y) - BaseRadius});
}
std::vector<Cell> Chessboard::neighbors(Cell p) const {
    std::vector<Cell> result;
    for (Cell next :
         {Cell{p.x, p.y - 1}, Cell{p.x, p.y + 1}, Cell{p.x - 1, p.y}, Cell{p.x + 1, p.y}})
        if (contains(next))
            result.push_back(next);
    return result;
}
std::vector<Cell> Chessboard::around(Cell p, int range) const {
    std::vector<Cell> result;
    for (int y = std::max(0, p.y - range); y <= std::min(rows_ - 1, p.y + range); ++y)
        for (int x = std::max(0, p.x - range); x <= std::min(columns_ - 1, p.x + range); ++x)
            result.push_back({x, y});
    return result;
}
Chessboard::Chessboard(int columns, int rows, std::vector<int> terrain,
                       std::array<BasePlacement, 2> bases)
    : columns_(columns), rows_(rows), terrain_(std::move(terrain)), bases_(bases) {
    if (columns <= 0 || rows <= 0 || columns > 1080 || rows > 1080 ||
        static_cast<std::size_t>(columns) * rows != terrain_.size() || terrain_.size() > 1000000)
        throw std::invalid_argument("Invalid battlefield dimensions");
    if (std::any_of(terrain_.begin(), terrain_.end(), [](int t) { return t < 0 || t > 9; }))
        throw std::invalid_argument("Invalid terrain type");
    baseIndex_.assign(size(), -1);
    for (int i = 0; i < 2; ++i) {
        const auto &base = bases_[i];
        if (teamIndex(base.team) != i || !contains(base.center) || base.center.x < BaseRadius ||
            base.center.y < BaseRadius || base.center.x + BaseRadius >= columns ||
            base.center.y + BaseRadius >= rows)
            throw std::invalid_argument("Invalid base placement");
        for (Cell p : around(base.center, BaseRadius)) {
            const int k = index(p);
            if (terrain_[k] != 0 || baseIndex_[k] != -1)
                throw std::invalid_argument("Overlapping or obstructed base");
            baseIndex_[k] = i;
        }
    }
    for (const auto &base : bases_)
        if (!walkable(base.spawn))
            throw std::invalid_argument("Invalid spawn cell");
    if (bases_[0].spawn == bases_[1].spawn)
        throw std::invalid_argument("Bases must have distinct spawn cells");
}
Chessboard Chessboard::parse(std::istream &map, std::istream &bases) {
    std::string line;
    if (!std::getline(map, line))
        throw std::invalid_argument("Missing map size");
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    std::size_t used = 0;
    const int size = std::stoi(line, &used);
    if (used != line.size() || size <= 0 || size > 1000000)
        throw std::invalid_argument("Invalid map size");
    int columns = 0, rows = 0;
    std::vector<int> terrain;
    while (std::getline(map, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty() || line.find_first_not_of("0123456789") != std::string::npos ||
            (columns && static_cast<int>(line.size()) != columns))
            throw std::invalid_argument("Invalid map row");
        columns = static_cast<int>(line.size());
        ++rows;
        if (terrain.size() + line.size() > static_cast<std::size_t>(size))
            throw std::invalid_argument("Too many map cells");
        for (char c : line)
            terrain.push_back(c - '0');
    }
    if (static_cast<int>(terrain.size()) != size)
        throw std::invalid_argument("Map size mismatch");
    std::array<BasePlacement, 2> placements{};
    for (int i = 0; i < 2; ++i) {
        long long team, center, spawn;
        if (!(bases >> team >> center >> spawn) || team != i + 1 || center < 0 || center >= size ||
            spawn < 0 || spawn >= size || columns == 0)
            throw std::invalid_argument("Invalid base configuration");
        placements[i] = {static_cast<Team>(team),
                         {static_cast<int>(center) % columns, static_cast<int>(center) / columns},
                         {static_cast<int>(spawn) % columns, static_cast<int>(spawn) / columns}};
    }
    if (bases >> line)
        throw std::invalid_argument("Unexpected base data");
    return Chessboard(columns, rows, std::move(terrain), placements);
}
Chessboard Chessboard::load(const std::filesystem::path &mapPath,
                            const std::filesystem::path &basePath) {
    std::ifstream map(mapPath), bases(basePath);
    if (!map || !bases)
        throw std::runtime_error("Cannot open map/base files");
    return parse(map, bases);
}
} // namespace skirmish
