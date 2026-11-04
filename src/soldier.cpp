#include "soldier.h"
#include "globalParameters.h"
#include <algorithm>

namespace skirmish {
Tick movementPeriod(int speed) {
    if (speed <= 0 || speed > static_cast<int>(TicksPerSecond))
        throw std::invalid_argument("Invalid movement speed");
    return (TicksPerSecond + static_cast<Tick>(speed) - 1) / static_cast<Tick>(speed);
}
UnitStats SoldierFactory::stats(UnitKind kind) {
    switch (kind) {
    case UnitKind::Infantry:
        return {INFANTRY_HP,    INFANTRY_ATK1,  INFANTRY_ATK2,
                INFANTRY_ATKRG, INFANTRY_MVSPD, INFANTRY_VSNRG};
    case UnitKind::Archer:
        return {ARCHER_HP, ARCHER_ATK1, ARCHER_ATK2, ARCHER_ATKRG, ARCHER_MVSPD, ARCHER_VSNRG};
    case UnitKind::SiegeGun:
        return {SIEGEGUN_HP,    SIEGEGUN_ATK1,  SIEGEGUN_ATK2,
                SIEGEGUN_ATKRG, SIEGEGUN_MVSPD, SIEGEGUN_VSNRG};
    }
    throw std::invalid_argument("Unknown unit kind");
}
Soldier SoldierFactory::create(UnitId id, UnitKind kind, Team team, Cell position, int hp) {
    teamIndex(team);
    const auto value = stats(kind);
    if (id == 0 || hp == 0 || hp < -1 || hp > value.maxHP)
        throw std::invalid_argument("Invalid unit identity/health");
    return {id,          team,
            kind,        position,
            value,       hp < 0 ? value.maxHP : hp,
            1,           1,
            0,           {position, position, 0, 0},
            std::nullopt};
}
} // namespace skirmish
