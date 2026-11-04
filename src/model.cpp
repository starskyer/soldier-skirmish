#include "model.h"
#include "attackBehavior.h"
#include <algorithm>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>

namespace skirmish {
const char *statusName(MatchStatus status) {
    switch (status) {
    case MatchStatus::Ready:
        return "ready";
    case MatchStatus::Running:
        return "running";
    case MatchStatus::Paused:
        return "paused";
    case MatchStatus::RedWon:
        return "red-won";
    case MatchStatus::BlueWon:
        return "blue-won";
    case MatchStatus::Draw:
        return "draw";
    case MatchStatus::Stopped:
        return "stopped";
    }
    throw std::invalid_argument("Unknown match status");
}
Model::Model(Chessboard board, SimulationOptions options)
    : board_(std::move(board)), options_(options), state_(board_), vision_(board_),
      spawnRandom_(options.seed), conflictRandom_(options.seed ^ 0x9e3779b9u) {
    if (!options.spawnPeriod)
        throw std::invalid_argument("Spawn period must be positive");
    vision_.refresh(board_, state_);
    assertInvariants();
}
UnitId Model::insert(UnitKind kind, Team team, Cell position, int hp) {
    if (!board_.walkable(position) || state_.occupancy.at(board_.index(position)))
        throw std::invalid_argument("Deployment cell is occupied");
    if (nextId_ == std::numeric_limits<UnitId>::max())
        throw std::overflow_error("Unit IDs exhausted");
    const auto id = nextId_;
    auto unit = SoldierFactory::create(id, kind, team, position, hp);
    unit.nextMove = state_.tick + 1;
    unit.nextAttack = state_.tick + 1;
    state_.units.emplace(id, unit);
    state_.occupancy[board_.index(position)] = id;
    ++nextId_;
    return id;
}
UnitId Model::deploy(UnitKind kind, Team team, Cell position, int hp) {
    if (state_.status != MatchStatus::Ready)
        throw std::logic_error("Scenario deployment is only allowed before start");
    const auto id = insert(kind, team, position, hp);
    vision_.refresh(board_, state_);
    assertInvariants();
    return id;
}
void Model::start() {
    if (state_.status == MatchStatus::Ready)
        state_.status = MatchStatus::Running;
}
void Model::togglePause() {
    if (state_.status == MatchStatus::Running)
        state_.status = MatchStatus::Paused;
    else if (state_.status == MatchStatus::Paused)
        state_.status = MatchStatus::Running;
}
void Model::stop() {
    if (!finished(state_.status))
        state_.status = MatchStatus::Stopped;
}
bool Model::step() {
    if (state_.status != MatchStatus::Running)
        return false;
    if (state_.tick == std::numeric_limits<Tick>::max())
        throw std::overflow_error("Simulation clock exhausted");
    // Everything in this turn reads one immutable, owning snapshot.
    BattleState snapshot = state_;
    ++snapshot.tick;
    std::vector<Intent> intents;
    intents.reserve(snapshot.units.size() + 2);
    for (const auto &entry : snapshot.units)
        intents.push_back(AttackBehavior::planUnit(board_, snapshot, entry.second));
    if (options_.baseAttacks)
        for (const auto &base : snapshot.bases)
            intents.push_back(AttackBehavior::planBase(snapshot, base));
    const auto resolved = resolveTurn(board_, snapshot, intents, conflictRandom_, conflictDraws_);
    state_.tick = snapshot.tick;
    for (const auto &action : intents)
        if (action.actor.kind == EntityKind::Unit) {
            auto &unit = state_.units.at(action.actor.id);
            unit.target = action.target;
            if (action.waitingForMove)
                ++unit.waitingTicks;
            else
                unit.waitingTicks = 0;
        }
    // Damage is accumulated before any removal; valid simultaneous shots survive their actor's
    // death.
    for (const auto &hit : resolved.unitDamage) {
        auto &unit = state_.units.at(hit.first);
        unit.hp = static_cast<int>(
            std::max<std::int64_t>(0, static_cast<std::int64_t>(unit.hp) - hit.second));
    }
    for (int i = 0; i < 2; ++i)
        state_.bases[i].hp = static_cast<int>(std::max<std::int64_t>(
            0, static_cast<std::int64_t>(state_.bases[i].hp) - resolved.baseDamage[i]));
    for (auto it = state_.units.begin(); it != state_.units.end();) {
        if (it->second.hp == 0) {
            state_.occupancy[board_.index(it->second.position)] = 0;
            it = state_.units.erase(it);
        } else
            ++it;
    }
    for (const auto actor : resolved.attacks) {
        if (actor.kind == EntityKind::Base)
            state_.bases[actor.id - 1].nextAttack = state_.tick + TicksPerSecond;
        else {
            const auto found = state_.units.find(actor.id);
            if (found != state_.units.end()) {
                found->second.nextAttack = state_.tick + TicksPerSecond;
                found->second.nextMove =
                    std::max(found->second.nextMove, state_.tick + TicksPerSecond);
            }
        }
    }
    for (const auto &move : resolved.moves)
        if (state_.units.count(move.unit))
            state_.occupancy[board_.index(move.from)] = 0;
    for (const auto &move : resolved.moves) {
        const auto found = state_.units.find(move.unit);
        if (found == state_.units.end())
            continue; // A killed mover never reappears.
        auto &unit = found->second;
        if (state_.occupancy[board_.index(move.to)])
            throw std::logic_error("Resolver produced conflicting moves");
        const auto duration = movementPeriod(unit.stats.moveSpeed);
        unit.position = move.to;
        unit.waitingTicks = 0;
        unit.motion = {move.from, move.to, state_.tick, state_.tick + duration};
        unit.nextMove = state_.tick + duration;
        unit.nextAttack = std::max(unit.nextAttack, state_.tick + duration);
        state_.occupancy[board_.index(move.to)] = unit.id;
    }
    const bool redDead = state_.bases[0].hp == 0, blueDead = state_.bases[1].hp == 0;
    if (redDead && blueDead)
        state_.status = MatchStatus::Draw;
    else if (redDead)
        state_.status = MatchStatus::BlueWon;
    else if (blueDead)
        state_.status = MatchStatus::RedWon;
    if (!finished(state_.status) && options_.autoSpawn)
        for (auto &base : state_.bases)
            if (state_.tick >= base.nextSpawn) {
                if (state_.occupancy[board_.index(base.spawn)] == 0)
                    insert(static_cast<UnitKind>(boundedRandom(spawnRandom_, 3, spawnDraws_)),
                           base.team, base.spawn);
                base.nextSpawn = state_.tick + options_.spawnPeriod;
            }
    for (auto &entry : state_.units) {
        auto &target = entry.second.target;
        if (target &&
            ((target->kind == EntityKind::Unit && !state_.units.count(target->id)) ||
             (target->kind == EntityKind::Base && state_.bases.at(target->id - 1).hp == 0)))
            target.reset();
    }
    vision_.refresh(board_, state_);
    assertInvariants();
    return true;
}
void Model::assertInvariants() const {
    if (state_.occupancy.size() != static_cast<std::size_t>(board_.size()))
        throw std::logic_error("Occupancy size mismatch");
    std::vector<UnitId> expected(board_.size(), 0);
    for (const auto &entry : state_.units) {
        const auto &unit = entry.second;
        if (!unit.id || unit.id != entry.first || unit.id >= nextId_ || unit.hp <= 0 ||
            unit.hp > unit.stats.maxHP || !board_.walkable(unit.position))
            throw std::logic_error("Invalid unit state");
        teamIndex(unit.team);
        auto &cell = expected[board_.index(unit.position)];
        if (cell)
            throw std::logic_error("Multiple units in one cell");
        cell = unit.id;
    }
    if (expected != state_.occupancy)
        throw std::logic_error("Unit registry and occupancy disagree");
    for (const auto &base : state_.bases)
        if (base.hp < 0 || base.hp > base.maxHP)
            throw std::logic_error("Invalid base health");
    if (finished(state_.status)) {
        const auto expectedStatus =
            state_.bases[0].hp == 0
                ? (state_.bases[1].hp == 0 ? MatchStatus::Draw : MatchStatus::BlueWon)
                : MatchStatus::RedWon;
        if (expectedStatus != state_.status || (state_.bases[0].hp > 0 && state_.bases[1].hp > 0))
            throw std::logic_error("Invalid result");
    }
}
std::string Model::fingerprint() const {
    std::uint64_t hash = 14695981039346656037ull;
    const auto add = [&](std::uint64_t value) {
        for (int i = 0; i < 8; ++i) {
            hash ^= value & 255;
            hash *= 1099511628211ull;
            value >>= 8;
        }
    };
    add(options_.seed);
    add(options_.autoSpawn);
    add(options_.baseAttacks);
    add(options_.spawnPeriod);
    add(board_.columns());
    add(board_.rows());
    for (int i = 0; i < board_.size(); ++i)
        add(board_.terrain(board_.cell(i)));
    add(state_.tick);
    add(static_cast<int>(state_.status));
    add(nextId_);
    add(spawnDraws_);
    add(conflictDraws_);
    for (const auto &base : state_.bases) {
        add(static_cast<int>(base.team));
        add(board_.index(base.center));
        add(board_.index(base.spawn));
        add(base.hp);
        add(base.nextAttack);
        add(base.nextSpawn);
    }
    for (const auto &entry : state_.units) {
        const auto &unit = entry.second;
        add(unit.id);
        add(static_cast<int>(unit.team));
        add(static_cast<int>(unit.kind));
        add(board_.index(unit.position));
        add(unit.hp);
        add(unit.nextMove);
        add(unit.nextAttack);
        add(unit.waitingTicks);
        add(board_.index(unit.motion.from));
        add(board_.index(unit.motion.to));
        add(unit.motion.begin);
        add(unit.motion.end);
        add(unit.target ? unit.target->id : 0);
        add(unit.target ? static_cast<int>(unit.target->kind) : 0);
    }
    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(16) << hash;
    return out.str();
}
} // namespace skirmish
