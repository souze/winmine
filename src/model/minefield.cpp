#include "Minefield.h"

#include <numeric>
#include <array>
#include <iostream>
#include <vector>

#include "../lib/util.h"

using util::Pos;

namespace {

void remove_erase(std::vector<int>& vec, int elem) {
    vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end());
}

void show_all_bombs(std::vector<Cell>& cells) {
    for (Cell& c : cells) {
        if (c.is_bomb()) {
            c.expose();
        }
    }
}

} // end anonymous namespace

void Minefield::random_place_bombs(Pos clicked_pos) {
    int clicked_index = clicked_pos.y * width + clicked_pos.x;

    std::vector<int> available_indices;
    available_indices.resize(field.size());
    std::iota(available_indices.begin(), available_indices.end(), 0);
    remove_erase(available_indices, clicked_index);

    for (int i = 0; i < num_bombs; ++i) {
        int mine_index = rand() % available_indices.size();
        field[available_indices[mine_index]].make_bomb();
        remove_erase(available_indices, available_indices[mine_index]);
    }
}

int Minefield::count_adjacent_bombs(int index) {
    Pos const pos{ index % width, index / width };

    std::vector<Pos> adjacent_positions = util::get_adjacent_positions(pos, width, height);

    return std::count_if(adjacent_positions.begin(), adjacent_positions.end(), [this](Pos p) {
        return get_cell(p).is_bomb();
        });
}

void Minefield::initialize_num_adjacent_bombs() {
    for (int i = 0; i < field.size(); ++i) {
        field[i].set_num_adjacent_bombs(count_adjacent_bombs(i));
    }
}

Minefield::Minefield(util::GameSettings game_settings)
    : width{ game_settings.width }
    , height{ game_settings.height }
    , num_bombs{ game_settings.num_bombs }
    , state{ GameState::Uninitialized }
    , field{ width * height }
{}

Minefield::Minefield(int width, int height, std::vector<Pos> const& mine_locations)
    : width{ width }
    , height{ height }
    , num_bombs{ static_cast<int>(mine_locations.size()) }
    , state{ GameState::Playing }
    , field{ width * height }
{
    for (Pos const& pos : mine_locations) {
        field[pos.y * width + pos.x].make_bomb();
    }
    initialize_num_adjacent_bombs();
}

Minefield& Minefield::operator=(Minefield& rhs) {
    using std::swap;
    swap(*this, rhs);
    return *this;
}

Cell& Minefield::get_cell(Pos const& pos) {
    return field[pos.y * width + pos.x];
}

Cell const& Minefield::get_cell(Pos const& pos) const {
    return field[pos.y * width + pos.x];
}

bool Minefield::check_win_condition() {
    return count_exposed_cells() == width * height - num_bombs;
}

void Minefield::expose(Pos pos) {

    if (state == GameState::Uninitialized) {
        random_place_bombs(pos);
        initialize_num_adjacent_bombs();
        state = GameState::Playing;
    }

    Cell& cell = get_cell(pos);
    if (cell.is_bomb()) {
        std::cout << "you lost\n";
        state = GameState::Lost;
        show_all_bombs(field);
    }
    else if (cell.is_covered()) {
        cell.expose();
        if (cell.get_num_adjacent_bombs() == 0) {
            for (Pos const& pos : util::get_adjacent_positions(pos, width, height)) {
                expose(pos);
            }
        }

        if (state == GameState::Playing && check_win_condition()) {
            state = GameState::Won;
            std::cout << "You have won!\n";
        }
    }
}

int Minefield::count_exposed_cells() {
    return std::count_if(field.begin(), field.end(), [](Cell const& cell) {
        return cell.is_exposed();
        });
}

void Minefield::toggle_flagged(Pos pos) {
    field[pos.y * width + pos.x].toggle_flagged();
}
void Minefield::make_flagged(Pos pos) {
    if (get_cell(pos).state == CellState::Covered) {
        get_cell(pos).state = CellState::Flagged;
    }
}

CellIter Minefield::begin() const {
    return CellIter(field.begin(), width);
}

CellIter Minefield::end() const {
    return CellIter(field.end(), width);
}

void swap(Minefield& lhs, Minefield& rhs) {
    using std::swap;
    swap(lhs.width, rhs.width);
    swap(lhs.height, rhs.height);
    swap(lhs.field, rhs.field);
    swap(lhs.state, rhs.state);
    swap(lhs.num_bombs, rhs.num_bombs);
}
