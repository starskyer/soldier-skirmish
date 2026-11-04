#include "turnResolver.h"
#include <algorithm>
#include <cstdlib>

namespace skirmish {
std::uint32_t boundedRandom(std::mt19937 &random, std::uint32_t bound, std::uint64_t &draws) {
    if (!bound)
        throw std::invalid_argument("Empty random choice");
    const std::uint32_t threshold = (0u - bound) % bound;
    std::uint32_t value;
    do {
        value = static_cast<std::uint32_t>(random());
        ++draws;
    } while (value < threshold);
    return value % bound;
}
Resolution resolveTurn(const Chessboard &board, const BattleState &state,
                       const std::vector<Intent> &intents, std::mt19937 &random,
                       std::uint64_t &draws) {
    Resolution result;
    std::map<std::pair<EntityKind, std::uint64_t>, int> counts;
    for (const auto &action : intents)
        ++counts[{action.actor.kind, action.actor.id}];
    std::map<int, std::vector<UnitId>> destinations;
    for (const auto &action : intents) {
        if (counts[{action.actor.kind, action.actor.id}] != 1) {
            ++result.rejected;
            continue;
        }
        if (action.kind == ActionKind::Wait)
            continue;
        const Soldier *unit = nullptr;
        const Base *base = nullptr;
        Team team = Team::Red;
        if (action.actor.kind == EntityKind::Unit) {
            const auto found = state.units.find(action.actor.id);
            if (found == state.units.end() || found->second.hp <= 0) {
                ++result.rejected;
                continue;
            }
            unit = &found->second;
            team = unit->team;
            if (action.from != unit->position ||
                state.occupancy.at(board.index(unit->position)) != unit->id) {
                ++result.rejected;
                continue;
            }
        } else {
            if (action.actor.id < 1 || action.actor.id > 2) {
                ++result.rejected;
                continue;
            }
            base = &state.bases[action.actor.id - 1];
            team = base->team;
            if (base->hp <= 0 || action.from != base->center) {
                ++result.rejected;
                continue;
            }
        }
        if (action.kind == ActionKind::Move) {
            if (!unit || state.tick < unit->nextMove || !board.walkable(action.destination) ||
                std::abs(unit->position.x - action.destination.x) +
                        std::abs(unit->position.y - action.destination.y) !=
                    1 ||
                state.occupancy.at(board.index(action.destination)) != 0) {
                ++result.rejected;
                continue;
            }
            destinations[board.index(action.destination)].push_back(unit->id);
            continue;
        }
        if (action.kind != ActionKind::Attack || !action.target ||
            state.tick < (unit ? unit->nextAttack : base->nextAttack)) {
            ++result.rejected;
            continue;
        }
        const auto target = *action.target;
        int damage = 0;
        if (target.kind == EntityKind::Unit) {
            const auto found = state.units.find(target.id);
            if (found == state.units.end() || found->second.hp <= 0 || found->second.team == team) {
                ++result.rejected;
                continue;
            }
            const auto position = unit ? unit->position : base->center;
            damage = unit ? unit->stats.unitDamage : base->damage;
            if (damage <= 0 || squareDistance(position, found->second.position) >
                                   (unit ? unit->stats.attackRange : base->range)) {
                ++result.rejected;
                continue;
            }
            result.unitDamage[target.id] += damage;
        } else {
            if (!unit || target.id < 1 || target.id > 2) {
                ++result.rejected;
                continue;
            }
            const auto &other = state.bases[target.id - 1];
            damage = unit->stats.baseDamage;
            if (other.hp <= 0 || other.team == team || damage <= 0 ||
                board.distanceToBase(unit->position, other.team) > unit->stats.attackRange) {
                ++result.rejected;
                continue;
            }
            result.baseDamage[target.id - 1] += damage;
        }
        result.attacks.push_back(action.actor);
    }
    for (auto &entry : destinations) {
        auto &ids = entry.second;
        const auto maxWait = std::max_element(ids.begin(), ids.end(), [&](UnitId a, UnitId b) {
            return state.units.at(a).waitingTicks < state.units.at(b).waitingTicks;
        });
        const Tick waiting = state.units.at(*maxWait).waitingTicks;
        ids.erase(
            std::remove_if(ids.begin(), ids.end(),
                           [&](UnitId id) { return state.units.at(id).waitingTicks != waiting; }),
            ids.end());
        std::sort(ids.begin(), ids.end());
        const auto winner =
            ids.size() == 1
                ? ids.front()
                : ids[boundedRandom(random, static_cast<std::uint32_t>(ids.size()), draws)];
        result.moves.push_back({winner, state.units.at(winner).position, board.cell(entry.first)});
    }
    return result;
}
} // namespace skirmish
