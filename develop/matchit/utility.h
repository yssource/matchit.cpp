#ifndef MATCHIT_UTILITY_H
#define MATCHIT_UTILITY_H

#include <variant>
#include <any>

namespace matchit
{
    namespace impl
    {

        template <typename T>
        constexpr auto cast = [](auto &&input)
        {
            return static_cast<T>(input);
        };

        constexpr auto deref = [](auto &&x) -> decltype(*x) & { return *x; };
        constexpr auto some = [](auto const pat)
        {
            return and_(app(cast<bool>, true), app(deref, pat));
        };

        constexpr auto none = app(cast<bool>, false);

        template <typename Value, typename Variant, typename = std::void_t<>>
        struct ViaGetIf : std::false_type
        {
        };

        using std::get_if;

        template <typename T, typename Variant>
        struct ViaGetIf<T, Variant, std::void_t<decltype(get_if<T>(std::declval<Variant const*>()))>>
            : std::true_type
        {
        };

        template <typename T, typename Variant>
        constexpr auto viaGetIfV = ViaGetIf<T, Variant>::value;

        static_assert(viaGetIfV<int, std::variant<int, bool>>);

        template <typename T>
        class AsPointer
        {
        public:
            template <typename Variant, std::enable_if_t<viaGetIfV<T, Variant>>* = nullptr>
            constexpr auto operator()(Variant const &v) const
            {
                return get_if<T>(std::addressof(v));
            }
            constexpr auto operator()(std::any const &a) const
            {
                return std::any_cast<T>(std::addressof(a));
            }
            template <typename B, std::enable_if_t<!viaGetIfV<T, B>>* = nullptr>
            constexpr auto operator()(B const &b) const
            -> decltype(dynamic_cast<T const *>(std::addressof(b)))
            {
                return dynamic_cast<T const *>(std::addressof(b));
            }
        };

        template <typename T>
        constexpr AsPointer<T> asPointer;

        template <typename T>
        constexpr auto as = [](auto const pat)
        {
            return app(asPointer<T>, some(pat));
        };

        template <typename Value, typename Pattern>
        constexpr auto matched(Value &&v, Pattern &&p)
        {
            return match(std::forward<Value>(v))(
                pattern | std::forward<Pattern>(p) = []
                { return true; },
                pattern | _ = []
                { return false; });
        }

        constexpr auto dsVia = [](auto&&... members)
        {
            return [members...](auto&&... pats)
            {
                return and_(app(members, pats)...);
            };
        };

        template <typename T>
        constexpr auto asDsVia = [](auto&&... members)
        {
            return [members...](auto&&... pats)
            {
                return as<T>(and_(app(members, pats)...));
            };
        };

    } // namespace impl
    using impl::as;
    using impl::matched;
    using impl::none;
    using impl::some;
    using impl::asDsVia;
} // namespace matchit

#endif // MATCHIT_UTILITY_H
