#include "attackBehavior.h"
#include "frameClock.h"
#include "model.h"
#include "pathfinding.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>

using namespace skirmish;
namespace {
void check(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}
template <class F> void rejects(F action) {
    bool caught = false;
    try {
        action();
    } catch (const std::exception &) {
        caught = true;
    }
    check(caught, "Expected rejection");
}
Chessboard arena() {
    return Chessboard(12, 8, std::vector<int>(96),
                      {{{Team::Red, {1, 1}, {1, 3}}, {Team::Blue, {10, 6}, {10, 4}}}});
}
SimulationOptions quiet(std::uint32_t seed = 1) {
    SimulationOptions options;
    options.autoSpawn = false;
    options.baseAttacks = false;
    options.seed = seed;
    return options;
}
Intent move(UnitId id, Cell from, Cell to) {
    return {EntityRef::unit(id), ActionKind::Move, from, to, std::nullopt, true};
}
Intent attack(UnitId actor, Cell from, EntityRef target) {
    return {EntityRef::unit(actor), ActionKind::Attack, from, from, target, false};
}
Resolution resolve(const Chessboard &board, const BattleState &state,
                   const std::vector<Intent> &actions, std::uint32_t seed = 1) {
    std::mt19937 random(seed);
    std::uint64_t draws = 0;
    return resolveTurn(board, state, actions, random, draws);
}
} // namespace
int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Expected asset directory\n";
        return 1;
    }
    const std::filesystem::path assets = std::filesystem::u8path(argv[1]);
    const std::vector<std::pair<const char *, std::function<void()>>> tests = {
        {"map parsing and edge queries",
         [&] {
             auto board = Chessboard::load(assets / "map.txt", assets / "base.txt");
             check(board.size() == 400 && board.columns() == 20, "Map dimensions");
             check(board.neighbors({0, 0}).size() == 2, "Corner neighbors");
             check(board.around({1, 1}, 0).size() == 1, "Center must not be duplicated");
             check(!board.walkable({-1, 0}) && !board.walkable({20, 20}), "Bounds before indexing");
             rejects([&] { board.index({-1, 0}); });
             rejects([&] { board.cell(400); });
         }},
        {"invalid maps and base layouts",
         [] {
             std::istringstream shortMap("400\n000\n"), bases("1\n21\n61\n2\n378\n338\n");
             rejects([&] { Chessboard::parse(shortMap, bases); });
             auto layouts = arena().placements();
             layouts[1].center = {2, 1};
             rejects([&] { Chessboard(12, 8, std::vector<int>(96), layouts); });
             layouts = arena().placements();
             layouts[0].spawn = layouts[0].center;
             rejects([&] { Chessboard(12, 8, std::vector<int>(96), layouts); });
             rejects([&] { Chessboard(0, 8, {}, layouts); });
         }},
        {"CRLF input and trailing data",
         [] {
             std::string map = "96\r\n";
             for (int y = 0; y < 8; ++y)
                 map += "000000000000\r\n";
             std::istringstream source(map), bases("1 13 37 2 82 58\r\n");
             check(Chessboard::parse(source, bases).size() == 96, "CRLF parse");
             std::istringstream source2(map), extra("1 13 37 2 82 58 extra");
             rejects([&] { Chessboard::parse(source2, extra); });
         }},
        {"factory and deployment ownership",
         [] {
             Model model(arena(), quiet());
             auto id = model.deploy(UnitKind::Infantry, Team::Red, {4, 3});
             check(model.state().units.at(id).hp == 80, "Original infantry stats");
             rejects([&] { model.deploy(UnitKind::Archer, Team::Blue, {4, 3}); });
             rejects([&] { model.deploy(UnitKind::Archer, Team::Blue, {1, 1}); });
             rejects([&] { model.deploy(UnitKind::Infantry, Team::Red, {5, 3}, 0); });
             model.start();
             rejects([&] { model.deploy(UnitKind::Infantry, Team::Red, {5, 3}); });
             model.assertInvariants();
         }},
        {"bounded BFS and legal next step",
         [&] {
             auto board = Chessboard::load(assets / "map.txt", assets / "base.txt");
             Model model(board, quiet());
             const auto id = model.deploy(UnitKind::Infantry, Team::Red, {1, 3});
             const auto goal = Cell{18, 16};
             auto state = model.state();
             auto unit = state.units.at(id);
             int steps = 0;
             while (unit.position != goal && steps < 400) {
                 auto next = findNextStep(board, state, unit, [&](Cell p) { return p == goal; });
                 check(next.has_value(), "Reachable goal");
                 check(std::abs(next->x - unit.position.x) + std::abs(next->y - unit.position.y) ==
                           1,
                       "Adjacent step");
                 state.occupancy[board.index(unit.position)] = 0;
                 unit.position = *next;
                 state.occupancy[board.index(*next)] = id;
                 ++steps;
             }
             check(steps == 30, "Default shortest path");
             check(!findNextStep(board, state, unit, [&](Cell p) { return p == unit.position; }),
                   "Already at goal");
             check(!findNextStep(board, state, unit, [&](Cell p) { return p == Cell{0, 6}; }),
                   "Wall is unreachable");
         }},
        {"initial vision is computed",
         [] {
             Model model(arena(), quiet());
             const auto &b = model.board();
             check(model.vision().visible(1, b.index({1, 1})), "Initial own base vision");
             check(!model.vision().visible(1, b.index({10, 6})), "No uninitialized enemy vision");
             check(model.vision().visible(0, b.index({10, 6})), "Merged team visions");
         }},
        {"longer waiting wins cell conflict",
         [] {
             Model model(arena(), quiet());
             auto a = model.deploy(UnitKind::Infantry, Team::Red, {4, 3});
             auto b = model.deploy(UnitKind::Infantry, Team::Red, {6, 3});
             auto state = model.state();
             state.tick = 1;
             state.units.at(b).waitingTicks = 20;
             auto result =
                 resolve(model.board(), state, {move(a, {4, 3}, {5, 3}), move(b, {6, 3}, {5, 3})});
             check(result.moves.size() == 1 && result.moves[0].unit == b, "Waiting priority");
         }},
        {"seeded conflicts ignore intent enumeration order",
         [] {
             Model model(arena(), quiet());
             auto a = model.deploy(UnitKind::Infantry, Team::Red, {4, 3});
             auto b = model.deploy(UnitKind::Infantry, Team::Red, {6, 3});
             auto state = model.state();
             state.tick = 1;
             const std::vector<Intent> actions = {move(a, {4, 3}, {5, 3}), move(b, {6, 3}, {5, 3})};
             auto reversed = actions;
             std::reverse(reversed.begin(), reversed.end());
             std::set<UnitId> winners;
             for (unsigned seed = 0; seed < 30; ++seed) {
                 auto first = resolve(model.board(), state, actions, seed);
                 auto second = resolve(model.board(), state, reversed, seed);
                 check(first.moves[0].unit == second.moves[0].unit,
                       "Order changed conflict result");
                 winners.insert(first.moves[0].unit);
             }
             check(winners.size() == 2, "Not a hard-coded ID winner");
         }},
        {"stale moves, occupied cells and duplicate actors rejected",
         [] {
             Model model(arena(), quiet());
             auto a = model.deploy(UnitKind::Infantry, Team::Red, {4, 3});
             auto b = model.deploy(UnitKind::Infantry, Team::Blue, {5, 3});
             (void)b;
             auto state = model.state();
             state.tick = 1;
             check(resolve(model.board(), state, {move(a, {4, 3}, {5, 3})}).moves.empty(),
                   "Cannot overwrite occupant");
             check(resolve(model.board(), state, {move(a, {3, 3}, {4, 4})}).moves.empty(),
                   "Stale origin");
             check(resolve(model.board(), state, {move(a, {4, 3}, {8, 3})}).moves.empty(),
                   "Cannot teleport");
             auto action = move(a, {4, 3}, {4, 4});
             check(resolve(model.board(), state, {action, action}).moves.empty(),
                   "Duplicate action");
         }},
        {"attacks bind identity rather than a reused cell",
         [] {
             Model model(arena(), quiet());
             auto a = model.deploy(UnitKind::Infantry, Team::Red, {4, 3});
             auto b = model.deploy(UnitKind::Infantry, Team::Blue, {5, 3});
             auto ally = model.deploy(UnitKind::Infantry, Team::Red, {4, 4});
             auto state = model.state();
             state.tick = 1;
             auto shot = attack(a, {4, 3}, EntityRef::unit(b));
             state.units.erase(b);
             state.occupancy[model.board().index({4, 4})] = 0;
             state.units.at(ally).position = {5, 3};
             state.occupancy[model.board().index({5, 3})] = ally;
             check(resolve(model.board(), state, {shot}).unitDamage.empty(),
                   "Reused cell must not redirect shot");
             shot.target = EntityRef::unit(ally);
             check(resolve(model.board(), state, {shot}).unitDamage.empty(),
                   "Friendly fire rejected");
         }},
        {"attacks revalidate range and cooldown",
         [] {
             Model model(arena(), quiet());
             auto a = model.deploy(UnitKind::Infantry, Team::Red, {4, 3});
             auto b = model.deploy(UnitKind::Infantry, Team::Blue, {8, 3});
             auto state = model.state();
             state.tick = 1;
             check(resolve(model.board(), state, {attack(a, {4, 3}, EntityRef::unit(b))})
                       .unitDamage.empty(),
                   "Out-of-range target");
             state.tick = 0;
             check(resolve(model.board(), state, {attack(a, {4, 3}, EntityRef::base(Team::Blue))})
                           .baseDamage[1] == 0,
                   "Cooldown check");
         }},
        {"simultaneous lethal shots both apply",
         [] {
             Model model(arena(), quiet());
             model.deploy(UnitKind::Infantry, Team::Red, {4, 3}, 15);
             model.deploy(UnitKind::Infantry, Team::Blue, {5, 3}, 15);
             model.start();
             model.step();
             check(model.state().units.empty(), "Both simultaneous attackers must die");
             model.assertInvariants();
         }},
        {"damage accumulates and follows the moving entity",
         [] {
             Model model(arena(), quiet());
             auto a = model.deploy(UnitKind::Infantry, Team::Red, {4, 3});
             auto ally = model.deploy(UnitKind::Infantry, Team::Red, {5, 4});
             auto b = model.deploy(UnitKind::Archer, Team::Blue, {5, 3});
             auto state = model.state();
             state.tick = 1;
             auto result =
                 resolve(model.board(), state,
                         {attack(a, {4, 3}, EntityRef::unit(b)),
                          attack(ally, {5, 4}, EntityRef::unit(b)), move(b, {5, 3}, {6, 3})});
             check(result.unitDamage.at(b) == 30, "Concurrent hits must accumulate");
             check(result.moves.size() == 1 && result.moves[0].unit == b,
                   "Moving target keeps its identity");
         }},
        {"killed mover never returns to the board",
         [] {
             Model model(arena(), quiet());
             auto mover = model.deploy(UnitKind::SiegeGun, Team::Red, {4, 3}, 15);
             model.deploy(UnitKind::Infantry, Team::Blue, {5, 3});
             model.start();
             model.step();
             check(!model.state().units.count(mover), "Killed mover removed");
             check(std::find(model.state().occupancy.begin(), model.state().occupancy.end(),
                             mover) == model.state().occupancy.end(),
                   "No ghost occupancy");
         }},
        {"one-second base attack cooldown",
         [] {
             auto options = quiet();
             options.baseAttacks = true;
             Model model(arena(), options);
             const auto id = model.deploy(UnitKind::Infantry, Team::Red, {8, 6});
             model.start();
             model.step();
             check(model.state().units.at(id).hp == 30, "Base damage is 50");
             for (int i = 0; i < 59; ++i)
                 model.step();
             check(model.state().units.at(id).hp == 30, "No unthrottled base attacks");
             model.step();
             check(!model.state().units.count(id), "Second attack after 60 ticks");
         }},
        {"siege guns keep building-only policy",
         [] {
             Model model(arena(), quiet());
             auto siege = model.deploy(UnitKind::SiegeGun, Team::Red, {4, 3});
             model.deploy(UnitKind::Infantry, Team::Blue, {5, 3});
             auto state = model.state();
             state.tick = 1;
             const auto action =
                 AttackBehavior::planUnit(model.board(), state, state.units.at(siege));
             check(action.target && action.target->kind == EntityKind::Base,
                   "Siege targets buildings");
             check(action.kind != ActionKind::Attack, "Cannot shoot adjacent soldier");
         }},
        {"pause freezes simulation and stop is terminal",
         [] {
             Model model(arena());
             check(!model.step(), "Ready state must not advance");
             model.start();
             model.step();
             model.togglePause();
             const auto before = model.fingerprint();
             for (int i = 0; i < 100; ++i)
                 check(!model.step(), "Paused step");
             check(model.fingerprint() == before, "Paused time/RNG changed");
             model.togglePause();
             check(model.step(), "Resume");
             model.stop();
             const auto stopped = model.fingerprint();
             model.start();
             model.togglePause();
             check(!model.step() && model.fingerprint() == stopped, "Stopped simulation restarted");
         }},
        {"spawning is bounded and new units wait until next tick",
         [] {
             Model model(arena());
             model.start();
             model.step();
             check(model.state().units.size() == 2, "Initial spawns");
             for (const auto &entry : model.state().units)
                 check(entry.second.position ==
                           model.board().placements()[teamIndex(entry.second.team)].spawn,
                       "No birth-tick action");
             model.assertInvariants();
         }},
        {"blocked spawn is never overwritten",
         [] {
             auto b = arena();
             std::vector<int> terrain(b.size());
             for (Cell p : {Cell{0, 3}, Cell{2, 3}, Cell{1, 4}})
                 terrain[b.index(p)] = 1;
             Model model(Chessboard(12, 8, terrain, b.placements()));
             auto occupant = model.deploy(UnitKind::SiegeGun, Team::Red, {1, 3});
             model.start();
             for (int i = 0; i < 310; ++i)
                 model.step();
             check(model.state().occupancy[model.board().index({1, 3})] == occupant,
                   "Spawn overwrote an existing unit");
             model.assertInvariants();
         }},
        {"both bases destroyed means draw and no more actions",
         [] {
             Model model(arena(), quiet());
             model.deploy(UnitKind::SiegeGun, Team::Red, {8, 6});
             model.deploy(UnitKind::SiegeGun, Team::Blue, {3, 1});
             model.start();
             for (int i = 0; i < 1000 && !finished(model.state().status); ++i)
                 model.step();
             check(model.state().status == MatchStatus::Draw, "Simultaneous base destruction");
             const auto result = model.fingerprint();
             check(!model.step() && model.fingerprint() == result, "Finished match advanced");
         }},
        {"same seed reproduces a complete default battle",
         [&] {
             auto board = Chessboard::load(assets / "map.txt", assets / "base.txt");
             Model a(board), b(board);
             a.start();
             b.start();
             for (int i = 0; i < 18000 && !finished(a.state().status); ++i) {
                 a.step();
                 b.step();
                 if (i % 30 == 0)
                     check(a.fingerprint() == b.fingerprint(), "Replay diverged");
             }
             check(finished(a.state().status) && a.fingerprint() == b.fingerprint(),
                   "Default battle must terminate consistently");
             std::cout << "  default battle: " << statusName(a.state().status) << " at tick "
                       << a.state().tick << " hash " << a.fingerprint() << '\n';
         }},
        {"clock decouples update from frame grouping", [] {
             FixedStepClock fine, coarse;
             int a = 0, b = 0;
             for (int i = 0; i < 120; ++i)
                 a += fine.consume(1.0 / 120, true);
             for (int i = 0; i < 30; ++i)
                 b += coarse.consume(1.0 / 30, true);
             check(a == 60 && b == 60, "Frame rate changed simulated time");
             FixedStepClock paused;
             paused.consume(0.01, true);
             const auto alpha = paused.alpha();
             check(paused.consume(100, false) == 0 && paused.alpha() == alpha,
                   "Pause changes interpolation phase");
             check(paused.consume(1.0 / 60 - 0.01, true) == 1, "Resume retained fractional tick");
             FixedStepClock stalled;
             check(stalled.consume(100, true) == 8, "Catch-up must be bounded");
             check(stalled.consume(0, true) == 0, "Huge backlog retained");
             rejects([&] { stalled.consume(-1, true); });
         }}};
    int failures = 0;
    for (const auto &test : tests) {
        try {
            test.second();
            std::cout << "PASS " << test.first << '\n';
        } catch (const std::exception &error) {
            ++failures;
            std::cerr << "FAIL " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << tests.size() << " scenarios, " << failures << " failures\n";
    return failures ? 1 : 0;
}
