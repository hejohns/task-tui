#pragma once
#include "model.hpp"

#include <ctime>
#include <ftxui/component/component.hpp>

#include <boost/stl_interfaces/iterator_interface.hpp>
#include <boost/stl_interfaces/view_adaptor.hpp>
#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>

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
        std::ranges::input_range R, // again, shouldn't matter viewable or not?
        std::regular_invocable<std::ranges::range_reference_t<R>> F
            >
    requires std::movable<F>
    class TaskTransform_view : public std::ranges::view_interface<TaskTransform_view<R, F>> {
        static_assert(std::ranges::constant_range<std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<R>>>>, "Does this ever fail?");
    public:
        // helper interface to inherit from
        class TaskTransform_iter : public boost::stl_interfaces::iterator_interface<
                                       std::forward_iterator_tag,
                                       std::invoke_result_t<F, std::ranges::range_reference_t<R>>
                                       > {
        public:
            // iterators are required to be default constructible
            TaskTransform_iter() = default;
            TaskTransform_iter(std::ranges::const_iterator_t<const R&> iter, const F& f) : iter_(iter), f_(f) {}
            constexpr auto operator*() const { return f_(*iter_); }
            constexpr auto& operator++() { ++iter_; return *this; }
            constexpr auto operator==(TaskTransform_iter other) const { return iter_ == other.iter_; }
            // this is rquired by c++'s method lookup rules, as described in the boost documentation
            // ++(int) (postfix ++)
            using boost::stl_interfaces::iterator_interface<std::forward_iterator_tag, std::invoke_result_t<F, std::ranges::range_reference_t<R>>>::operator++;
        private:
            // wraps iterator to input range
            std::ranges::const_iterator_t<const R&> iter_;
            F f_;
        };
        static_assert(std::forward_iterator<TaskTransform_iter>);
    public:
        // NOTE: it was considered a defect against c++20 that views had to be default constructible
        //TaskTransform_view() = default;
        // if rg is a constant_range, then this is already fine, and could take by reference even
        TaskTransform_view(const R& rg, const F& f) : begin_(std::ranges::cbegin(rg), f), end_(std::ranges::cend(rg), f) {}
        constexpr auto begin() const { return begin_; }
        constexpr auto end() const { return end_; }
    private:
        TaskTransform_iter begin_;
        TaskTransform_iter end_;
    };

    inline constexpr auto lam = [](int x) noexcept -> int { return 0; };
    //template<typename R>
    //concept CC = requires(R r){
    //    { TaskTransform_view<R, decltype(lam)>(r, lam)/*.begin()*/ };
    //};
    //static_assert(CC<const std::vector<int> &>);
    static_assert(std::same_as<std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<std::vector<int>>>>, const std::vector<int> &>);
    static_assert(std::same_as<std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<const std::vector<int>>>>, const std::vector<int> &>);
    static_assert(std::same_as<std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<const std::vector<int>&>>>, const std::vector<int> &>);
    static_assert(std::ranges::constant_range<std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<const std::vector<int>>>>>);
    //static_assert(TaskTransform_view<const std::vector<int>&, decltype(lam)>::TaskTransform_iter);
    static_assert(std::forward_iterator<TaskTransform_view<const std::vector<int>&, decltype([](int x)noexcept->int{return 0;})>::TaskTransform_iter>);

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
         * I don't think we care if the range is viewable or not, even thought it would make the const handling slightly easier?
         * NOTE: I don't think we really care if F is regular_invocable or just
         * invocable, and c++23 concepts can't tell us the difference
         */
        template<
            std::ranges::input_range R,
            std::regular_invocable<std::ranges::range_reference_t<R>> F
                >
        requires
            std::convertible_to<std::ranges::range_reference_t<R>, int&/*task_tui::Task&*/> // just for an example
            and std::movable<F>
        // use const R & to remove the need to worry about forwarding/const views/constant ranges
        auto operator()(const R& rg, const F& f) const { // const method because we say this is a constexpr below
            return TaskTransform_view<R, std::remove_const_t<F>>(rg, f);
        }
    };

    // idiom to make this available
    inline constexpr auto task_transform = boost::stl_interfaces::adaptor(TaskTransform{});
}
