#include "controller.h"

#include <functional>
#include <chrono>
#include <thread>

#include "../model/minefield.h"
#include "../solver/solver.h"
#include "../lib/util.h"

using util::Pos;

void Controller::update_view() {
    if (update_view_callback) {
        update_view_callback(minefield);
    }
}

void Controller::expose(util::Pos pos) {
    std::cout << "Exposing " << pos << '\n';
    minefield.expose(pos);
    update_view();
}

void Controller::toggle_flagged(Pos pos) {
    minefield.toggle_flagged(pos);
    update_view();
}

void Controller::new_game(util::GameSettings game_settings) {
    minefield = Minefield{ game_settings };
    update_view();
}

void Controller::auto_one_move() {
    auto const& moves = solver::find_best_moves(minefield);
    if (!moves.empty()) {
        expose(moves.back());
    }
    else {
        expose({ rand() % minefield.get_width(), rand() % minefield.get_height() });
    }
}

void Controller::auto_play(std::chrono::milliseconds delay) {
    while (!minefield.is_game_lost() && !minefield.is_game_won()) {
        solver::board_state_result result = solver::explore_possible_minefield_states(minefield);
        if (!result.safest_positions.empty()) {
            expose(result.safest_positions.back());
        }
        else {
            expose({ rand() % minefield.get_width(), rand() % minefield.get_height() });
        }
        if (result.unsafe_certainty > .99) {
            flag_positions(result.unsafest_positions);
        }
        std::this_thread::sleep_for(delay);
    }
}

void Controller::flag_positions(std::vector<util::Pos> const& positions) {
    for (util::Pos pos : positions) {
        minefield.make_flagged(pos);
    }
}

void Controller::auto_flag_bombs() {
    flag_positions(solver::find_bombs(minefield));
    update_view();
}

void Controller::set_update_view_callback(std::function<void(Minefield const&)> cb) {
    std::cout << "Setting update view CB\n";
    update_view_callback = cb;
}