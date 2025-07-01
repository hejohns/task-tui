#pragma once
#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <vector>

#include <boost/stl_interfaces/iterator_interface.hpp>
#include <boost/stl_interfaces/view_adaptor.hpp>

namespace task_tui {
    namespace ranges {
        /*
         * This is essentially a reimplementation of
         * std::ranges::views::transform, to learn how to use c++23 ranges and
         * range adaptors
         *
         * The range adaptor entry-point is `inline constexpr auto map`
         * (the code is in dependency order, ie buttom-up)
         */
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
         * If we didn't use boost::stl_interfaces::adaptor,
         * map_range_adaptor_closure would have to inherit the
         * std::ranges::range_adaptor_closure tag itself, in order to be
         * pipeable as a range adaptor
         *
         * (To resolve a syntactic ambiguity inherent with pipes, the language
         * standards hacked in the requirement that to be pipeable, you have to
         * satisfy the RangeAdaptor(Closure)Object concept, which among other
         * things requires you to "opt-in" by tagging your callable with
         * std::ranges::range_adaptor_closure)
         */
        struct map_range_adaptor_closure /*: public std::ranges::range_adaptor_closure<map_range_adaptor_closure>*/ {
            /*
             * I don't think we care if the range R is viewable or not, even
             * though it might make the const handling slightly easier by
             * allowing us to use std::ranges::views::all
             *
             * I don't think we really care if F is regular_invocable rather
             * than just invocable, and c++23 concepts can't even differentiate
             */
            template<
                std::ranges::input_range R,
                std::regular_invocable<std::ranges::range_reference_t<R>> F
                    >
            // so that the iterators are std::forward iterators, which requires that the iterators are std::movable
            // (modeled after std::ranges::views::transform)
            requires std::movable<F>
            // we could use universal/forwarding references, but there's no point since we want map to be const
            auto operator()(const R& rg, const F& f) const { // const method because map is a constexpr below
                return TaskTransform_view(rg, f);
            }
        };

        /* Range adaptor idiom to have nice pipeable syntax that is also
         * equivalent to "direct" application (as required of range adaptors)
         *
         * We use the boost stl_interfaces adaptor to reduce the boilerplate,
         * which I don't think has a c++23 standard equivalent yet
         */
        inline constexpr auto map = boost::stl_interfaces::adaptor(map_range_adaptor_closure{});

        // tests
        constexpr int abc(){
            std::vector<int> v = {1, 2, 3, 4, 5};
            return 0;
        }
        static_assert(abc());
    }
}
