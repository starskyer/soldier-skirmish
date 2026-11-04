#include "attackBehavior.h"
#include "pathfinding.h"
#include <limits>

namespace skirmish {
namespace {
const Soldier *nearestEnemy(const BattleState &state, const Soldier &actor, int range) {
    const Soldier *selected = nullptr;
    int best = std::numeric_limits<int>::max();
    for (const auto &entry : state.units) {
        const auto &other = entry.second;
        const int distance = squareDistance(actor.position, other.position);
        if (other.hp > 0 && other.team != actor.team && distance <= range && distance < best) {
            selected = &other;
            best = distance;
        }
    }
    return selected;
}
} // namespace
Intent AttackBehavior::planUnit(const Chessboard &board, const BattleState &state,
                                const Soldier &unit) {
    Intent action{EntityRef::unit(unit.id),
                  ActionKind::Wait,
                  unit.position,
                  unit.position,
                  std::nullopt,
                  false};
    const auto *target =
        unit.stats.unitDamage > 0 ? nearestEnemy(state, unit, unit.stats.attackRange) : nullptr;
    if (target) {
        action.target = EntityRef::unit(target->id);
        if (state.tick >= unit.nextAttack)
            action.kind = ActionKind::Attack;
        return action;
    }
    target =
        unit.stats.unitDamage > 0 ? nearestEnemy(state, unit, unit.stats.visionRange) : nullptr;
    const Team enemy = opponent(unit.team);
    if (target)
        action.target = EntityRef::unit(target->id);
    else {
        action.target = EntityRef::base(enemy);
        if (board.distanceToBase(unit.position, enemy) <= unit.stats.attackRange) {
            if (state.tick >= unit.nextAttack)
                action.kind = ActionKind::Attack;
            return action;
        }
    }
    if (state.tick < unit.nextMove)
        return action;
    const auto next = findNextStep(board, state, unit, [&](Cell p) {
        return target ? squareDistance(p, target->position) <= unit.stats.attackRange
                      : board.distanceToBase(p, enemy) <= unit.stats.attackRange;
    });
    action.waitingForMove = true;
    if (next) {
        action.kind = ActionKind::Move;
        action.destination = *next;
    }
    return action;
}
Intent AttackBehavior::planBase(const BattleState &state, const Base &base) {
    Intent action{EntityRef::base(base.team),
                  ActionKind::Wait,
                  base.center,
                  base.center,
                  std::nullopt,
                  false};
    if (base.hp <= 0 || state.tick < base.nextAttack)
        return action;
    int best = std::numeric_limits<int>::max();
    for (const auto &entry : state.units) {
        const auto &unit = entry.second;
        const int distance = squareDistance(base.center, unit.position);
        if (unit.hp > 0 && unit.team != base.team && distance <= base.range && distance < best) {
            best = distance;
            action.target = EntityRef::unit(unit.id);
            action.kind = ActionKind::Attack;
        }
    }
    return action;
}
} // namespace skirmish
