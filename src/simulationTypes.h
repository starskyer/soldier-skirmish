#pragma once
#include "globalParameters.h"
#include <cstdint>
#include <optional>
#include <stdexcept>

namespace skirmish {
using UnitId = std::uint64_t;
using Tick = std::uint64_t;
constexpr Tick TicksPerSecond = 60;
static_assert(BASE_SIZE > 0 && BASE_SIZE % 2 == 1, "Base size must be positive and odd");
static_assert(BASE_GENTIME > 0, "Spawn period must be positive");
constexpr int BaseRadius = BASE_SIZE / 2;
struct Cell {
    int x = 0, y = 0;
    friend bool operator==(Cell a, Cell b) { return a.x == b.x && a.y == b.y; }
    friend bool operator!=(Cell a, Cell b) { return !(a == b); }
};
enum class Team { Red = 1, Blue = 2 };
inline int teamIndex(Team team) {
    if (team != Team::Red && team != Team::Blue)
        throw std::invalid_argument("Invalid team");
    return static_cast<int>(team) - 1;
}
inline Team opponent(Team team) {
    return teamIndex(team) == 0 ? Team::Blue : Team::Red;
}
enum class UnitKind { Infantry, Archer, SiegeGun };
enum class EntityKind { Unit, Base };
struct EntityRef {
    EntityKind kind = EntityKind::Unit;
    std::uint64_t id = 0;
    static EntityRef unit(UnitId id) { return {EntityKind::Unit, id}; }
    static EntityRef base(Team team) {
        return {EntityKind::Base, static_cast<std::uint64_t>(team)};
    }
    friend bool operator==(EntityRef a, EntityRef b) { return a.kind == b.kind && a.id == b.id; }
};
enum class ActionKind { Wait, Move, Attack };
struct Intent {
    EntityRef actor;
    ActionKind kind = ActionKind::Wait;
    Cell from{}, destination{};
    std::optional<EntityRef> target;
    bool waitingForMove = false;
};
enum class MatchStatus { Ready, Running, Paused, RedWon, BlueWon, Draw, Stopped };
inline bool finished(MatchStatus status) {
    return status == MatchStatus::RedWon || status == MatchStatus::BlueWon ||
           status == MatchStatus::Draw;
}
const char *statusName(MatchStatus status);
int squareDistance(Cell a, Cell b);
Tick movementPeriod(int cellsPerSecond);
} // namespace skirmish
