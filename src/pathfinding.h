#pragma once
#include "battleState.h"
#include <functional>

namespace skirmish {
std::optional<Cell> findNextStep(const Chessboard &board, const BattleState &snapshot,
                                 const Soldier &unit, const std::function<bool(Cell)> &goal);
}
