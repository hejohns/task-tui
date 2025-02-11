#pragma once
#include "model.hpp"

#include <ftxui/component/component.hpp>

#include <boost/stl_interfaces/iterator_interface.hpp>
#include <concepts>
#include <ranges>

namespace task_tui {
    ///* create a DOM component which is a list of Tasks, from a suitable input range
    // *
    // * NOTE: this is a slight generalization over just requiring that the input
    // * range is of task_tui::Tasks, more for fun than of actual use
    // *
    // * TOOD: should input_range also be view/viewable_range?? I'm still confused about this
    //*/
    //template<std::ranges::input_range R>
    //requires std::convertible_to<std::ranges::range_reference_t<R>, task_tui::Task&>
    //ftxui::Components tasks(R&& rg){
    //    return rg |
    //        std::ranges::views::transform([](const Task& task) noexcept -> ftxui::Component {
    //                return ftxui::Button(task.description, [](){}, ftxui::ButtonOption::Simple());
    //                }) |
    //        std::ranges::to<ftxui::Components>();
    //}
    // This is essentially an eta expansion of transform?
    //              input                output
    template<
        std::ranges::input_range R,
        std::regular_invocable<std::ranges::range_reference_t<R>> F
            >
    requires
        std::move_constructible<F>
    // helper interface to inherit from
    class TaskTransform_iter : public boost::stl_interfaces::iterator_interface<
                                   std::forward_iterator_tag,
                                   std::invoke_result_t<F, std::ranges::range_reference_t<R>>
                                   > {
    public:
        // iterators are required to be default constructible
        TaskTransform_iter() = default;
        TaskTransform_iter(std::ranges::const_iterator_t<R> iter, const F& f) : iter_(iter), f_(f) {}
        constexpr auto operator*() const { return f_(*iter_); }
        constexpr auto& operator++() { ++iter_; return *this; }
        constexpr auto operator==(TaskTransform_iter<R, F> other) const { return iter_ == other.iter_; }
        // this is rquired by c++'s method lookup rules, as described in the boost documentation
        // ++(int) (postfix ++)
        using boost::stl_interfaces::iterator_interface<std::forward_iterator_tag, std::invoke_result_t<F, std::ranges::range_reference_t<R>>>::operator++;
    private:
        // wraps iterator to input range
        std::ranges::const_iterator_t<R> iter_;
        F f_;
    };
    static_assert(std::forward_iterator<TaskTransform_iter<const std::vector<int>, decltype([](int)->int{return 0;})>>);

    template<
        std::ranges::input_range R,
        std::regular_invocable<std::ranges::range_reference_t<R>> F
            >
    requires std::ranges::constant_range<std::add_const_t<R>>
    class TaskTransform_view : public std::ranges::view_interface<TaskTransform_view<R, F>> {
    public:
        // NOTE: it was considered a defect against c++20 that views had to be default constructible
        //TaskTransform_view() = default;
        //TaskTransform_view(R&& rg, F&& f) : begin_(std::ranges::begin(rg), std::forward<F>(f)), end_(std::ranges::end(rg), std::forward<F>(f)) {}
        // NOTE: this is where we need that views are copyable?
        TaskTransform_view(const R& rg, const F& f) : begin_(std::ranges::cbegin(rg), f), end_(std::ranges::cend(rg), f) {}
        constexpr auto begin() const { return begin_; }
        constexpr auto end() const { return end_; }
    private:
        TaskTransform_iter<R, F> begin_;
        TaskTransform_iter<R, F> end_;
    };

    /*
     * This is some syntactic sugar to make TaskTransform(_view) pipeable as a
     * range adaptor
     *
     * TaskTransform inherits from the tag (empty struct)
     * std::ranges::range_adaptor_closure just to satisfy the
     * RangeAdaptor(Closure)Object concept, so it is pipeable (|) as a range
     * adaptor. (This language standards hack is to resolve a syntactic
     * ambiguity inherent with pipes, and to explicitly say "this callable
     * should be pipeable, when used as such".)
     *
     * (And therefore, this needs to be a struct, not just any callable like a function.)
     */
    struct TaskTransform : public std::ranges::range_adaptor_closure<TaskTransform> {
        /*
         * TODO: should R additionally be a view?? Why would we really care?
         * NOTE: I don't think we really care if F is regular_invocable or just
         * invocable, and c++23 concepts can't tell us the difference
         */
        template<
            std::ranges::input_range R,
            std::regular_invocable<std::ranges::range_reference_t<R>> F
                >
        requires
            std::convertible_to<std::ranges::range_reference_t<R>, int&/*task_tui::Task&*/>
            and std::move_constructible<F>
            and std::ranges::constant_range<std::add_const_t<R>>
        // we could've just taken these as const &, but this is a universal reference here
        // just in the spirit of this being a wrapper, but it makes the template ugly
        auto operator()(R&& rg, F&& f) const { // because we say this is a constexpr below
            return TaskTransform_view(std::forward<R>(rg), std::forward<F>(f));
        }
    };

    // idiom to make this available
    inline constexpr TaskTransform task_transform;
}
