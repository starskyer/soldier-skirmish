#include "vision.h"
#include <algorithm>

namespace skirmish {
Vision::Vision(const Chessboard &board)
    : masks_{std::vector<unsigned char>(board.size()), std::vector<unsigned char>(board.size())} {}
void Vision::refresh(const Chessboard &board, const BattleState &state) {
    for (auto &mask : masks_)
        std::fill(mask.begin(), mask.end(), 0);
    for (const auto &base : state.bases)
        if (base.hp > 0)
            for (Cell p : board.around(base.center, base.range))
                masks_[teamIndex(base.team)][board.index(p)] = 1;
    for (const auto &entry : state.units) {
        const auto &unit = entry.second;
        if (unit.hp > 0)
            for (Cell p : board.around(unit.position, unit.stats.visionRange))
                masks_[teamIndex(unit.team)][board.index(p)] = 1;
    }
}
bool Vision::visible(int mode, int cell) const {
    if (mode < 0 || mode > 2)
        throw std::invalid_argument("Invalid vision mode");
    if (mode == 0)
        return masks_[0].at(cell) != 0 || masks_[1].at(cell) != 0;
    return masks_[mode - 1].at(cell) != 0;
}
} // namespace skirmish
