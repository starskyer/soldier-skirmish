#pragma once
#include "turnResolver.h"
#include "vision.h"
#include <string>

namespace skirmish {
struct SimulationOptions {
    std::uint32_t seed = 1;
    bool autoSpawn = true;
    bool baseAttacks = true;
    Tick spawnPeriod = BASE_GENTIME * TicksPerSecond;
};
class Model {
  public:
    explicit Model(Chessboard board, SimulationOptions options = {});
    UnitId deploy(UnitKind kind, Team team, Cell position, int hp = -1);
    void start();
    void togglePause();
    void stop();
    bool step();
    const Chessboard &board() const { return board_; }
    const BattleState &state() const { return state_; }
    const Vision &vision() const { return vision_; }
    void assertInvariants() const;
    std::string fingerprint() const;

  private:
    Chessboard board_;
    SimulationOptions options_;
    BattleState state_;
    Vision vision_;
    UnitId nextId_ = 1;
    std::mt19937 spawnRandom_, conflictRandom_;
    std::uint64_t spawnDraws_ = 0, conflictDraws_ = 0;
    UnitId insert(UnitKind kind, Team team, Cell position, int hp = -1);
};
} // namespace skirmish
