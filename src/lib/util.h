#pragma once

namespace util {

	struct Pos {
		int x = -1; // to detect uninitialized
		int y = -1; // to detect uninitialized
	};

	// TODO seems like this is a complicated solution to an easy problem
	inline std::vector<std::tuple<int, int>> get_adjacent_indices(int x, int y, int max_width, int max_height) {
		std::vector<std::tuple<int, int>> adjacent_indices;

		if (y - 1 >= 0) {
			if (x - 1 >= 0) adjacent_indices.emplace_back(x - 1, y - 1);
			adjacent_indices.emplace_back(x, y - 1);
			if (x + 1 < max_width) adjacent_indices.emplace_back(x + 1, y - 1);
		}
		if (x - 1 >= 0) adjacent_indices.emplace_back(x - 1, y);
		if (x + 1 < max_width) adjacent_indices.emplace_back(x + 1, y);
		if (y + 1 < max_height) {
			if (x - 1 >= 0) adjacent_indices.emplace_back(x - 1, y + 1);
			adjacent_indices.emplace_back(x, y + 1);
			if (x + 1 < max_width) adjacent_indices.emplace_back(x + 1, y + 1);
		}
		return adjacent_indices;
	}

} // namespace util