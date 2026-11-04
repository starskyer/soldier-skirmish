#pragma once
#include "battleState.h"

namespace skirmish {
class AttackBehavior {
  public:
    static Intent planUnit(const Chessboard &board, const BattleState &snapshot,
                           const Soldier &unit);
    static Intent planBase(const BattleState &snapshot, const Base &base);
};
} // namespace skirmish
