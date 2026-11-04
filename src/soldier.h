#pragma once
#include "simulationTypes.h"

namespace skirmish {
struct UnitStats {
    int maxHP, unitDamage, baseDamage, attackRange, moveSpeed, visionRange;
};
struct Motion {
    Cell from{}, to{};
    Tick begin = 0, end = 0;
};
struct Soldier {
    UnitId id = 0;
    Team team = Team::Red;
    UnitKind kind = UnitKind::Infantry;
    Cell position{};
    UnitStats stats{};
    int hp = 0;
    Tick nextMove = 1, nextAttack = 1, waitingTicks = 0;
    Motion motion{};
    std::optional<EntityRef> target;
};
class SoldierFactory {
  public:
    static UnitStats stats(UnitKind kind);
    static Soldier create(UnitId id, UnitKind kind, Team team, Cell position, int hp = -1);
};
} // namespace skirmish
