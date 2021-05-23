#include "gui.h"

void NewGameForm::place_components() {
    nana::place place{ *this };
    place.div("vert<textwidth><textheight><textmines><cancel><start><label>");

    place["textwidth"] << text_width;
    place["textheight"] << text_height;
    place["textmines"] << text_num_mines;
    place["cancel"] << btn_cancel;
    place["start"] << btn_start;
    place["label"] << label_msg;

    place.collocate();
}

void NewGameForm::start_game() {
    if (auto game_settings = get_settings()) {
        start_cb(game_settings.value());
        close();
    }
    else {
        label_msg.caption("Those are incorrect settings");
    }
}

std::optional<util::GameSettings> NewGameForm::get_settings()
{
    util::GameSettings gs;
    try {
        gs.width = std::stoi(text_width.text());
        gs.height = std::stoi(text_height.text());
        gs.num_bombs = std::stoi(text_num_mines.text());
    }
    catch (...) {
        return {};
    }
    return gs;
}

NewGameForm::NewGameForm(StartCallback start_cb)
    : text_width{ *this, "Width" }
    , text_height{ *this, "Height" }
    , text_num_mines{ *this, "# Mines" }
    , btn_start{ *this, "Start" }
    , btn_cancel{ *this, "Cancel" }
    , start_cb{ start_cb }
{
    place_components();

    btn_cancel.events().click([this]() {
        close();
        });

    btn_start.events().click([this]() {
        start_game();
        });

    show();
}

void Gui::show_new_game_dialog() {
    new_game_form = std::make_unique<NewGameForm>([this](util::GameSettings new_game_settings) {
        game_settings = new_game_settings;
        create_buttons();
        place_components();
        control->new_game(game_settings);
        });
    new_game_form->events().unload([this]() {
        new_game_form = nullptr;
        });
}

void Gui::fill_menu_bar() {
    nana::menu& game_item = menubar.push_back("&Game");
    game_item.append("Reset", [this](nana::menu::item_proxy&) {
        control->new_game(game_settings);
        });
    game_item.append("New game", [this](nana::menu::item_proxy&) {
        show_new_game_dialog();
        });

    nana::menu& solver_item = menubar.push_back("&Solver");
    solver_item.append("One move", [this](nana::menu::item_proxy&) {
        control->auto_one_move();
        });
    solver_item.append("Mark bombs", [this](nana::menu::item_proxy&) {
        control->auto_flag_bombs();
        });
    solver_item.append("Autoplay", [this](nana::menu::item_proxy&) {
        nana::threads::pool_push(thread_pool, [this]() {
            using namespace std::chrono_literals;
            control->auto_play(0ms);
            })();
        });
    solver_item.append("Autoplay with delay", [this](nana::menu::item_proxy&) {
        nana::threads::pool_push(thread_pool, [this]() {
            using namespace std::chrono_literals;
            control->auto_play(80ms);
            })();
        });
}

void Gui::create_buttons() {
    mine_buttons.clear();
    for (int x = 0, y = 0; y < game_settings.height;) {
        mine_buttons.emplace_back(std::make_unique<nana::button>(form, " "));
        mine_buttons.back()->events().mouse_up([this, x, y](nana::arg_mouse const& arg) {
            if (arg.button == nana::mouse::left_button) {
                control->expose({ x, y });
            }
            else if (arg.button == nana::mouse::right_button) {
                control->toggle_flagged({ x, y });
            }
            });
        y = x < game_settings.width - 1 ? y : y + 1;
        x = x < game_settings.width - 1 ? x + 1 : 0;
    }
}

void Gui::place_components() {
    form.size(nana::size(
        game_settings.width * cell_width_pixels,
        game_settings.height * cell_width_pixels + menubar_height + status_line_height));

    //Layout management
    nana::place place = nana::place{ form };
    place.div("vert<menubar weight=" + std::to_string(menubar_height) + ">"
        "<minefield "
        "grid=[" + std::to_string(game_settings.width) + "," + std::to_string(game_settings.height) + "]>"
        "vert<statusline weight=" + std::to_string(status_line_height) + ">"
    );
    place["menubar"] << menubar;
    for (auto const& cell : mine_buttons) {
        place["minefield"] << *cell;
    }
    place["statusline"] << status_line;
    place.collocate();
}

Gui::Gui(util::GameSettings settings, std::shared_ptr<Controller> control)
    : game_settings{ settings }
    , control{ control }
    , form{}
    , menubar{ form }
    , status_line{ form }
{
    fill_menu_bar();

    create_buttons();

    place_components();

    control->set_update_view_callback([this](Minefield const& mf) {
        show_minefield(mf);
        });
}

void Gui::start() {
    form.show();

    //Start to event loop process, it blocks until the form is closed.
    nana::exec();
}

// Callback used when the minefield has updated, and we need to show the new state

void Gui::show_minefield(Minefield const& minefield) {

    for (auto const& [pos, cell] : minefield) {
        nana::button& btn = *mine_buttons[pos.y * game_settings.width + pos.x];
        if (cell.is_exposed()) {
            if (cell.is_bomb()) {
                btn.caption("x");
            }
            else if (cell.get_num_adjacent_bombs() > 0) {
                btn.caption(std::to_string(cell.get_num_adjacent_bombs()));
            }
            else {
                btn.caption("_"); // pressed but no adjacent mines
            }
        }
        else if (cell.is_flagged()) {
            btn.caption("B");
        }
        else {
            btn.caption(" ");
        }
    }
    if (minefield.is_game_lost()) {
        status_line.caption("You lost");
    }
    else if (minefield.is_game_won()) {
        status_line.caption("You won");
    }
    else {
        status_line.caption("");
    }
}
