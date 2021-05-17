#pragma once

#include <numeric>
#include <array>

class Cell {
	bool exposed = false;
	bool marked_bomb = false;
	int adjacent_mines = 0;

public:
	void make_mine() {
		adjacent_mines = -1;
	}

	bool is_mine() const {
		return adjacent_mines == -1;
	}

	void expose() {
		exposed = true;
	}

	bool is_exposed() const {
		return exposed;
	}

	int num_adjacent_mines() const {
		return adjacent_mines;
	}

	void set_adjacent_mines(int num) {
		adjacent_mines = num;
	}

	void toggle_marked_bomb() {
		marked_bomb = !marked_bomb;
	}
	bool is_marked_bomb() const {
		return marked_bomb;
	}
};

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

	std::tuple<int, int, Cell> operator*() {
		return { inc_count % width, inc_count / width, *cell_iter };
	}

	void operator++() {
		++cell_iter;
		++inc_count;
	}
};

void remove_erase(std::vector<int>& vec, int elem) {
	vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end());
}

enum class GameState {
	Playing
	, Lost
	, Won
};

class Minefield {
	int const width = 0;
	int const height = 0;
	std::vector<Cell> field;
	bool initialized = false; // only initialize the mines after the first click/expose
	GameState state = GameState::Playing;
	int num_mines = 0;

	void place_mines(int clicked_x, int clicked_y) {
		int clicked_index = clicked_y * width + clicked_x;

		std::vector<int> available_indices;
		available_indices.resize(field.size());
		std::iota(available_indices.begin(), available_indices.end(), 0);
		remove_erase(available_indices, clicked_index);
		
		for (int i = 0; i < num_mines; ++i) {
			int mine_index = rand() % available_indices.size();
			field[available_indices[mine_index]].make_mine();
			remove_erase(available_indices, mine_index);
		}
	}

	std::tuple<int, int> get_x_y_from_index(int index) {
		return { index % width, index / width };
	}

	std::vector<int> get_adjacent_indices(int x, int y) {
		std::vector<int> adjacent_indices;
		int const index = y * width + x;

		if (y - 1 >= 0) {
			if (x - 1 >= 0) adjacent_indices.push_back(index - width - 1);
			adjacent_indices.push_back(index - width);
			if (x + 1 < width) adjacent_indices.push_back(index - width + 1);
		}
		if (x - 1 >= 0) adjacent_indices.push_back(index - 1);
		if (x + 1 < width) adjacent_indices.push_back(index + 1);
		if (y + 1 < height) {
			if (x - 1 >= 0) adjacent_indices.push_back(index + width - 1);
			adjacent_indices.push_back(index + width);
			if (x + 1 < width) adjacent_indices.push_back(index + width + 1);
		}
		return adjacent_indices;
	}

	// TODO seems like this is a complicated solution to an easy problem
	int calc_adjacent_mines(int index) {
		int const x = index % width;
		int const y = index / width;

		std::vector<int> adjacent_indices = get_adjacent_indices(x, y);

		auto is_valid_and_is_mine = [this](int index) {
			return index >= 0 && index < this->width * this->height && this->field[index].is_mine();
		};

		return std::count_if(adjacent_indices.begin(), adjacent_indices.end(), is_valid_and_is_mine);
	}

	void calculate_num_adjacent_mines() {
		for (int i = 0; i < field.size(); ++i) {
			if (!field[i].is_mine()) {
				field[i].set_adjacent_mines(calc_adjacent_mines(i));
			}
		}
	}

public:
	Minefield(int width, int height, int num_mines)
		: width{ width }
		, height{ height }
		, field{width*height}
		, num_mines{ num_mines }
	{}

	Minefield(Minefield&) = delete;
	
	Minefield(Minefield&& mf)
		: Minefield(mf.width, mf.height, mf.num_mines)
	{}

	Cell& get_cell(int x, int y) {
		return field[y * width + x];
	}

	void expose(int x, int y) {
		if (get_cell(x, y).is_exposed()) {
			return;
		}

		if (!initialized) {
			place_mines(x, y);
			calculate_num_adjacent_mines();
			initialized = true;
		}
		
		if (get_cell(x, y).is_mine()) {
			std::cout << "you lost\n";
			state = GameState::Lost;
			for (auto& cell : field) {
				if (cell.is_mine()) {
					cell.expose();
				}
			}
		}
		else {
			get_cell(x, y).expose();
			if (get_cell(x, y).num_adjacent_mines() == 0) {
				for (int i : get_adjacent_indices(x, y)) {
					this->expose(i % width, i / width);
				}
			}
		}

		if (count_exposed_cells() == width * height - num_mines) {
			state = GameState::Won;
			std::cout << "You have won!\n";
		}
	}

	int count_exposed_cells() {
		return std::count_if(field.begin(), field.end(), [](Cell const& cell) {
			return cell.is_exposed();
			});
	}

	void toggle_marked_bomb(int x, int y) {
		field[y * width + x].toggle_marked_bomb();
	}

	CellIter begin() const {
		return CellIter(field.begin(), width);
	}

	CellIter end() const {
		return CellIter(field.end(), width);
	}
};