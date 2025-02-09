#include "model.hpp"

namespace task_tui {

    std::istream& operator>>(std::istream& is, Task& t){
        is >> t.description >> t.goal;
        decltype(t.due_date)::value_type due_date;
        std::chrono::from_stream(is, "{:%F}", due_date);
        t.due_date = due_date;
        return is;
    }

    std::ostream& operator<<(std::ostream& os, Task& t){
        return os << t.description << t.goal << t.due_date;
    }
}
