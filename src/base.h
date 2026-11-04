#pragma once
#include "chessboard.h"

namespace skirmish {
struct Base {
    Team team;
    Cell center, spawn;
    int hp, maxHP, damage, range;
    Tick nextAttack = 1, nextSpawn = 1;
    explicit Base(const BasePlacement &placement);
};
} // namespace skirmish
