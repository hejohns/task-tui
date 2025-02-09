#include "model.hpp"
#include "components.hpp"

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/util/ref.hpp>

#include <ranges>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iostream>

ftxui::Component Task(
        std::string task,
        std::string goal,
        std::optional<task_tui::date> due_date
        ){
    auto component = ftxui::Checkbox(ftxui::CheckboxOption::Simple());
    // cheating the interface a little but,
    // `struct CheckboxBase : ComponentBase, CheckboxOption` is internal to ftxui
    // so let's just cast when we need something
    auto component_as_checkboxoption = std::dynamic_pointer_cast<ftxui::CheckboxOption>(component);
    component_as_checkboxoption->label = std::format("[{}] {}", goal, task);
    return component;
}

// I would've rather used `open O_CREAT`, but I suppose this is technically more portable
std::fstream open_fstream(
        std::filesystem::path path,
        std::ios::openmode mode = std::ios::in)
{
    std::fstream fs(path, mode);
    if(!fs.is_open()){
        std::filesystem::create_directories(path.parent_path());
        fs.open(path, std::ios::out | std::ios::trunc);
        fs.open(path, mode);
    }
    return fs;
}

int main(){
    /* load (pending and completed) task, goal, and recurring_task data */
    std::filesystem::path task_tui_dir(std::format("{}/.task-tui/", std::getenv("HOME")));
    auto pending_tasks_fs = open_fstream(task_tui_dir / "pending_tasks");
    auto goals_fs = open_fstream(task_tui_dir / "goals");
    auto recurring_tasks_fs = open_fstream(task_tui_dir / "recurring_tasks");
    auto completed_tasks_fs = open_fstream(task_tui_dir / "completed_tasks");
    /* fire up the terminal UI */
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    /* construct the ftxui DOM components */
    auto pending_tasks = task_tui::tasks(std::ranges::views::istream<task_tui::Task>(pending_tasks_fs));
    ///* piece together the ftxui DOM */
    //// why does vscroll_indicator have to go before yframe?
    auto pending_tasks_component = ftxui::Container::Vertical(pending_tasks) | ftxui::vscroll_indicator | ftxui::yframe | ftxui::xflex;
    // TODO
    ftxui::Components goals;
    auto goals_component = ftxui::Container::Vertical(goals) | ftxui::flex;
    // TODO
    ftxui::Components recurring_tasks;
    auto recurring_tasks_component = ftxui::Container::Vertical(recurring_tasks) | ftxui::flex;
    // TODO
    ftxui::Components completed_tasks;
    auto completed_tasks_component = ftxui::Container::Vertical(completed_tasks) | ftxui::flex;
    int selected_tab = 0;
    int selected_horizontal_component = 0;
    auto main_component = ftxui::Container::Tab(
            { ftxui::Container::Horizontal(
                    { pending_tasks_component
                    , ftxui::Container::Vertical(
                            { goals_component
                            , recurring_tasks_component
                            })
                    }
                    , &selected_horizontal_component)
            , ftxui::Container::Vertical(completed_tasks)
            }
            , &selected_tab);
    screen.Loop(main_component);
    return 0;
}
