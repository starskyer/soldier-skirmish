#pragma once
#include "base.h"
#include "soldier.h"
#include <array>
#include <map>
#include <vector>

namespace skirmish {
struct BattleState {
    Tick tick = 0;
    MatchStatus status = MatchStatus::Ready;
    std::array<Base, 2> bases;
    std::map<UnitId, Soldier> units;
    std::vector<UnitId> occupancy;
    explicit BattleState(const Chessboard &board)
        : bases{Base(board.placements()[0]), Base(board.placements()[1])},
          occupancy(board.size(), 0) {}
};
} // namespace skirmish
