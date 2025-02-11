#pragma once
#include <string>
#include <chrono>

// use boost:optional (instead of std::optional), which has >> and <<
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

namespace task_tui {
    using date = std::chrono::year_month_day;

    //struct QuotedString {
    //    std::string str;
    //};

    //struct Goal {
    //    QuotedString short_description;
    //};

    //std::istream& operator>>(std::istream& is, Goal& t);
    //std::ostream& operator<<(std::ostream& os, Goal& t);

    //struct Task {
    //    Goal goal;
    //    QuotedString description;
    //};

    //struct PendingTask : public Task {
    //    boost::optional<date> due_date;
    //};

    //struct CompletedTask : public Task {
    //    date completed_date;
    //};

    //// boilerplate for (de)serializing Tasks
    //// SEE: std::ranges::views::istream
    //std::istream& operator>>(std::istream& is, Task& t);
    //std::istream& operator>>(std::istream& is, PendingTask& t);
    //std::istream& operator>>(std::istream& is, CompletedTask& t);
    //std::ostream& operator<<(std::ostream& os, Task& t);
    //std::ostream& operator<<(std::ostream& os, PendingTask& t);
    //std::ostream& operator<<(std::ostream& os, CompletedTask& t);
}
