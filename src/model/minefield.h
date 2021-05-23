#pragma once

#include <numeric>
#include <array>
#include <iostream>
#include <vector>

#include "../lib/util.h"

enum class CellState {
    Covered,
    Exposed,
    Flagged
};

// One square in a minefield
class Cell {
    int adjacent_bombs = 0;
    bool _is_bomb = false;

public:
    CellState state = CellState::Covered;

    bool is_exposed() const { return state == CellState::Exposed; }
    bool is_covered() const { return !is_exposed(); }
    bool is_flagged() const { return state == CellState::Flagged; }
    void expose() { state = CellState::Exposed; }

    void toggle_flagged() {
        switch (state) {
        case CellState::Covered: state = CellState::Flagged; break;
        case CellState::Flagged: state = CellState::Covered; break;
        }
    }

    bool is_bomb() const { return _is_bomb; }
    void make_bomb() { _is_bomb = true; }

    int get_num_adjacent_bombs() const { return adjacent_bombs; }
    void set_num_adjacent_bombs(int num) { adjacent_bombs = num; }
};


// Allows iterating over a minefield with a for-each loop
class CellIter {
    std::vector<Cell>::const_iterator cell_iter;
    int const width;
    int inc_count = 0;

public:
    CellIter(std::vector<Cell>::const_iterator iter, int width)
        : cell_iter{ iter }
        , width{ width }
        , inc_count{ 0 }
    {}

    bool operator!=(CellIter const& rhs) {
        return cell_iter != rhs.cell_iter;
    }

    std::tuple<util::Pos, Cell> operator*() {
        return { {inc_count % width, inc_count / width}, *cell_iter };
    }

    void operator++() {
        ++cell_iter;
        ++inc_count;
    }
};

enum class GameState {
    Uninitialized
    , Playing
    , Lost
    , Won
};

class Minefield {
    int width = 0;
    int height = 0;
    int num_bombs = 0;  // Needed because the bomb placement is deferred
    std::vector<Cell> field; // width*height size, flattened with index = y*width + x
    GameState state = GameState::Uninitialized; // only initialize the bombs after the first click/expose

    void random_place_bombs(util::Pos clicked_pos);

    int count_adjacent_bombs(int index);

    void initialize_num_adjacent_bombs();

    bool check_win_condition();

    Cell& get_cell(util::Pos const& pos);

public:
    Minefield(util::GameSettings game_settings);
    Minefield(int width, int height, std::vector<util::Pos> const& mine_locations);

    Minefield& operator=(Minefield& rhs);

    CellIter begin() const;
    CellIter end() const;

    friend void swap(Minefield& lhs, Minefield& rhs);

    int get_width() const { return width; }
    int get_height() const { return height; }
    int get_num_mines() const { return num_bombs; }
    Cell const& get_cell(util::Pos const& pos) const;

    bool is_game_lost() const { return state == GameState::Lost; }
    bool is_game_won() const { return state == GameState::Won; }

    void expose(util::Pos pos);

    int count_exposed_cells();

    void toggle_flagged(util::Pos pos);
    void make_flagged(util::Pos pos);
};