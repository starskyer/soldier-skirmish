#pragma once
#include "simulationTypes.h"
#include <array>
#include <filesystem>
#include <istream>
#include <vector>

namespace skirmish {
struct BasePlacement {
    Team team;
    Cell center;
    Cell spawn;
};
class Chessboard {
  public:
    Chessboard(int columns, int rows, std::vector<int> terrain, std::array<BasePlacement, 2> bases);
    static Chessboard parse(std::istream &map, std::istream &bases);
    static Chessboard load(const std::filesystem::path &map, const std::filesystem::path &bases);
    int columns() const { return columns_; }
    int rows() const { return rows_; }
    int size() const { return static_cast<int>(terrain_.size()); }
    bool contains(Cell cell) const;
    int index(Cell cell) const;
    Cell cell(int index) const;
    int terrain(Cell cell) const;
    int baseAt(Cell cell) const;
    bool walkable(Cell cell) const;
    int distanceToBase(Cell cell, Team team) const;
    std::vector<Cell> neighbors(Cell cell) const;
    std::vector<Cell> around(Cell cell, int range) const;
    const std::array<BasePlacement, 2> &placements() const { return bases_; }

  private:
    int columns_, rows_;
    std::vector<int> terrain_, baseIndex_;
    std::array<BasePlacement, 2> bases_;
};
} // namespace skirmish
