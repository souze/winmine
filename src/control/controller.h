#pragma once

#include <functional>
#include <chrono>
#include <thread>

#include "../model/minefield.h"
#include "../solver/solver.h"
#include "../lib/util.h"

class Controller {
    Minefield minefield;
    std::function<void(Minefield const&)> update_view_callback; // register to receive callback 
                                                                // when the minefield is updated

    void update_view();

public:
    Controller::Controller(Minefield&& m)
        : minefield{ std::move(m) }
    {}
    Controller(Controller&) = delete;
    
    Minefield const& get_minefield() { return minefield;  }

    void expose(util::Pos pos);

    void toggle_flagged(util::Pos pos);

    void new_game(util::GameSettings game_settings);

    void auto_one_move();
    void auto_play(std::chrono::milliseconds delay);
    void auto_flag_bombs();

    void flag_positions(std::vector<util::Pos> const& positions);

    void set_update_view_callback(std::function<void(Minefield const&)> cb);

};