#pragma once
#include "battleState.h"
#include <random>

namespace skirmish {
struct Move {
    UnitId unit;
    Cell from, to;
};
struct Resolution {
    std::vector<Move> moves;
    std::map<UnitId, std::int64_t> unitDamage;
    std::array<std::int64_t, 2> baseDamage{};
    std::vector<EntityRef> attacks;
    std::size_t rejected = 0;
};
std::uint32_t boundedRandom(std::mt19937 &random, std::uint32_t bound, std::uint64_t &draws);
Resolution resolveTurn(const Chessboard &board, const BattleState &snapshot,
                       const std::vector<Intent> &intents, std::mt19937 &random,
                       std::uint64_t &draws);
} // namespace skirmish
