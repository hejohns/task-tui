#pragma once
#include "model.hpp"

#include <ftxui/component/component.hpp>

#include <ranges>

namespace task_tui {
    /* create a DOM component which is a list of Tasks, from a suitable input range
     *
     * NOTE: this is a slight generalization over just requiring that the input
     * range is of task_tui::Tasks, more for fun than of actual use
     *
     * TOOD: should input_range also be view/viewable_range??
    */
    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, task_tui::Task&>
    ftxui::Components tasks(R&& rg){
        return rg |
            std::ranges::views::transform([](const Task& task) noexcept -> ftxui::Component {
                    return ftxui::Button(task.description, [](){}, ftxui::ButtonOption::Simple());
                    }) |
            std::ranges::to<ftxui::Components>();
    }
}
