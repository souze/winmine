#pragma once

#include <functional>
#include <chrono>
#include <optional>

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/menubar.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/threads/pool.hpp>

#include "../control/controller.h"

class NewGameForm : public nana::form {
	nana::textbox text_width{ *this, "Width" };
	nana::textbox text_height{ *this, "Height" };
	nana::textbox text_num_mines{ *this, "# Mines" };
	nana::button btn_start{ *this, "Start" };
	nana::button btn_cancel{ *this, "Cancel" };
    nana::label label_msg{ *this, "" };

	using StartCallback = std::function<void(util::GameSettings)>;
	StartCallback start_cb;

    void place_components();
    void start_game();
    std::optional<util::GameSettings> get_settings();

public:
    NewGameForm(StartCallback start_cb);
};

class Gui {
	static constexpr unsigned int cell_width_pixels = 20;
	static constexpr unsigned int menubar_height = 26;
	static constexpr unsigned int status_line_height = 20;

	nana::form form;
	nana::menubar menubar;
	std::vector<std::unique_ptr<nana::button>> mine_buttons;
	nana::label status_line;
	std::unique_ptr<NewGameForm> new_game_form;

	std::shared_ptr<Controller> control;
	nana::threads::pool thread_pool;
    util::GameSettings game_settings;

    void show_new_game_dialog();

    void fill_menu_bar();
    void create_buttons();
    void place_components();

public:
    Gui(util::GameSettings settings, std::shared_ptr<Controller> control);
	Gui(Gui&) = delete;

    void start();

	// Callback used when the minefield has updated, and we need to show the new state
    void show_minefield(Minefield const& minefield);

};