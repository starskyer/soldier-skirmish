#pragma once
#include "battleState.h"

namespace skirmish {
class Vision {
  public:
    explicit Vision(const Chessboard &board);
    void refresh(const Chessboard &board, const BattleState &state);
    bool visible(int mode, int cell) const;

  private:
    std::array<std::vector<unsigned char>, 2> masks_;
};
} // namespace skirmish
