#pragma once
#include <string>
#include <chrono>

// for >>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

namespace task_tui {
    using date = std::chrono::year_month_day;

    struct Task{
        std::string description;
        std::string goal;
        boost::optional<date> due_date;
    };

    // boilerplate for std::ranges::views::istream
    std::istream& operator>>(std::istream& is, Task& t);
    std::ostream& operator<<(std::ostream& os, Task& t);
}
