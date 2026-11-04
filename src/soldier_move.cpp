#include "pathfinding.h"
#include <queue>

namespace skirmish {
std::optional<Cell> findNextStep(const Chessboard &board, const BattleState &state,
                                 const Soldier &unit, const std::function<bool(Cell)> &goal) {
    if (!board.walkable(unit.position) ||
        state.occupancy.size() != static_cast<std::size_t>(board.size()))
        throw std::invalid_argument("Invalid pathfinding snapshot");
    const int start = board.index(unit.position);
    std::vector<int> parent(board.size(), -2);
    std::queue<int> frontier;
    parent[start] = -1;
    frontier.push(start);
    while (!frontier.empty()) {
        int current = frontier.front();
        frontier.pop();
        if (goal(board.cell(current))) {
            if (current == start)
                return std::nullopt;
            while (parent[current] != start)
                current = parent[current];
            return board.cell(current);
        }
        for (Cell next : board.neighbors(board.cell(current))) {
            const int n = board.index(next);
            if (board.walkable(next) && parent[n] == -2 &&
                (state.occupancy[n] == 0 || state.occupancy[n] == unit.id)) {
                parent[n] = current;
                frontier.push(n);
            }
        }
    }
    return std::nullopt;
}
} // namespace skirmish
