/**
 * Improved min() and max() that can handle references, rvalue-references, and mixed integral types.
 * @maintainer xavier dot roche at algolia.com
 */

#include <type_traits>
#include <utility>
#include <limits>

namespace enhancedminmax {

/** Helper: get highest unsigned version that can hold positive values of L and/or R ***/
template<typename L, typename R>
struct highest_unsigned_type
{
    static_assert(std::is_integral_v<L>);
    static_assert(std::is_integral_v<R>);

    static inline auto constexpr _helper()
    {
        // Choose the largest type, and make it unsigned.
        if constexpr (sizeof(L) >= sizeof(R)) {
            return std::make_unsigned_t<L>();
        } else {
            return std::make_unsigned_t<R>();
        }
    }

    using type = decltype(_helper());
};
template<typename L, typename R>
using highest_unsigned_type_t = typename highest_unsigned_type<L, R>::type;

/** Helper: return @c true if l is strictly lesser than r ***/
struct lower_than
{
    template<typename LL, typename RR>
    bool inline constexpr operator()(LL&& l, RR&& r) const
    {
        using L = std::decay_t<LL>;
        using R = std::decay_t<RR>;

        // Not an integral number: compare
        if constexpr (not std::is_integral_v<L> || not std::is_integral_v<R>) {
            return l < r;
        }
        // Same signedness: no issue
        else if constexpr (std::is_unsigned_v<L> == std::is_unsigned_v<R>) {
            return l < r;
        }
        // Left is signed; if negative we return true, otherwise, compare
        else if constexpr (std::is_signed_v<L> && std::is_unsigned_v<R>) {
            using H = highest_unsigned_type_t<L, R>;
            return l < 0 || static_cast<H>(l) < static_cast<H>(r);
        }
        // Right is signed; if negative we return false, otherwise, compare
        else if constexpr (std::is_unsigned_v<L> && std::is_signed_v<R>) {
            using H = highest_unsigned_type_t<L, R>;
            return r >= 0 && static_cast<H>(l) < static_cast<H>(r);
        }
        // Unreachable
        else {
            static_assert(not std::is_same_v<L, L>, "logic error"); // static_assert(false)
        }
    }
};

/** Helper: return @c true if r is strictly higher than l ***/
struct higher_than : private lower_than
{
    template<typename L, typename R>
    bool inline constexpr operator()(L&& l, R&& r) const
    {
        // Simply reverse the right<->left logic.
        return lower_than::operator()(r, l);
    }
};

/**
 * Return the maximum of a set of values. If the values are compatible, return the lvalue-reference, or
 * rvalue-reference, depending on the case.
 * @param less The comparison functor
 * @param is_signed Set to @c true if the selected value was originally a signed integral type; even if the
 * returned type is unsigned (because 'true ? -1 : 1' yields an unsigned type)
 * @return The lowest value, which may be an integral signed type value cast to unsigned if is_signed is set to @c
 * true.
 **/
template<typename Compare, typename L, typename... R>
decltype(auto) inline constexpr find_lowest(const Compare& less, bool& is_signed, L&& l, R&&... r)
{
    // Only one element to compare: return it.
    if constexpr (sizeof...(r) == 0) {
        // If handling integral types, propagate sign
        if constexpr (std::is_integral_v<L>) {
            is_signed = std::is_signed_v<L>;
        }
        return std::forward<L>(l);
    } else {
        // Find lowest of the right part, and return the sign.
        bool right_signed = false;
        auto&& right = find_lowest(less, right_signed, std::forward<R>(r)...);

        // The right part is an unsigned integral type, but was originally a signed value. Compare it with the
        // right type then.
        using RightType = std::decay_t<decltype(right)>;
        if constexpr (std::is_integral_v<RightType> && std::is_unsigned_v<RightType>) {
            if (right_signed) {
                using RealRightType = std::make_signed_t<RightType>;
                auto&& realRight = static_cast<RealRightType>(right);

                // Compare it with the right type
                const bool is_lower = less(std::forward<L>(l), realRight);
                is_signed = is_lower ? std::is_signed_v<L> : right_signed;

                // But return the expected type
                return is_lower ? std::forward<L>(l) : right;
            }
        }

        // Compare and return the value. Mark sign flag if needed for upstream to handle further comparisons.
        const bool is_lower = less(std::forward<L>(l), right);
        // If handling integral types, propagate sign
        if constexpr (std::is_integral_v<L>) {
            is_signed = is_lower ? std::is_signed_v<L> : right_signed;
        }
        return is_lower ? std::forward<L>(l) : right;
    }
}

/** Return the minimum of a set of values. If the values are compatible, return the lvalue-reference, or
 * rvalue-reference, depending on the case. ***/
template<typename... T>
decltype(auto) inline constexpr min(T&&... args)
{
    bool is_signed = false;
    return find_lowest(lower_than(), is_signed, std::forward<T>(args)...);
}

/** Return the maximum of a set of values. If the values are compatible, return the lvalue-reference, or
 * rvalue-reference, depending on the case. ***/
template<typename... T>
decltype(auto) inline constexpr max(T&&... args)
{
    bool is_signed = false;
    return find_lowest(higher_than(), is_signed, std::forward<T>(args)...);
}

/* Unit tests */
namespace ut {

static_assert(!higher_than()(0, 7u));
static_assert(higher_than()(7u, 0));
static_assert(higher_than()(7u, -2));
static_assert(!higher_than()(-2, 7u));

static_assert(max(0) == 0);
static_assert(max(0, 1) == 1);
static_assert(max(0, 1, 2, 3, 4, 5) == 5);
static_assert(max(1, 0) == 1);
static_assert(max(0u, 1) == 1);
static_assert(max(1u, 0) == 1);
static_assert(max(0, 1u) == 1);
static_assert(max(1, 0u) == 1);
static_assert(max(0, -1) == 0);
static_assert(std::is_signed_v<decltype(min(0, -1))>);
static_assert(max(-1, 0) == 0);
static_assert(max(0u, -1) == 0);
static_assert(std::is_unsigned_v<decltype(min(0u, -1))>);
static_assert(max(-1, 0u) == 0);
static_assert(max(0u, -2) == 0);
static_assert(max(2u, 0, 7u) == 7);
static_assert(max(2u, 0u, 7u) == 7);
static_assert(max(-2, 0u) == 0);

static_assert(max(-2, 0u, 7u) == 7);
static_assert(max(0, 7u, -2) == 7);
static_assert(max(0u, 7u, -2) == 7);
static_assert(max(7u, 0u, -2) == 7);
static_assert(max(std::numeric_limits<int>::min(), 0, 7) == 7);
static_assert(max(std::numeric_limits<unsigned long long>::min(), 0, 7) == 7);
static_assert(max(std::numeric_limits<signed long long>::min(), 0, 7) == 7);
static_assert(max(std::numeric_limits<signed long long>::max(), 0, 7) ==
              std::numeric_limits<signed long long>::max());
static_assert(max(std::numeric_limits<unsigned long long>::max(), 0, 7) ==
              std::numeric_limits<unsigned long long>::max());
static_assert(max(std::numeric_limits<unsigned long long>::max(), 0, 7, std::numeric_limits<int>::min()) ==
              std::numeric_limits<unsigned long long>::max());
static_assert(max(std::numeric_limits<unsigned long long>::max(),
                  0,
                  7,
                  std::numeric_limits<signed long int>::min()) == std::numeric_limits<unsigned long long>::max());
static_assert(max(std::numeric_limits<unsigned long long>::max(),
                  0,
                  7,
                  std::numeric_limits<signed long long>::min()) == std::numeric_limits<unsigned long long>::max());
static_assert(max(std::numeric_limits<unsigned long long>::max(),
                  0,
                  7,
                  std::numeric_limits<signed long long>::max()) == std::numeric_limits<unsigned long long>::max());

static_assert(min(0) == 0);
static_assert(min(0, 1) == 0);
static_assert(min(1, 0) == 0);
static_assert(min(0u, 1) == 0);
static_assert(min(1u, 0) == 0);
static_assert(min(0, 1u) == 0);
static_assert(min(1, 0u) == 0);
static_assert(min(0, -1) == -1);
static_assert(min(-1, 0) == -1);
static_assert(min(0u, -1) == -1);
static_assert(min(-1, 0u) == -1);
static_assert(min(0u, -2) == -2);
static_assert(min(2u, 0, 7u) == 0);
static_assert(min(2u, 0u, 7u) == 0);
static_assert(min(-2, 0u) == -2);

static_assert(min(-2, 0u, 7u) == -2);
static_assert(min(0u, 7u, -2) == -2);
static_assert(min(7u, 0u, -2) == -2);
static_assert(min(-2, 0u, 7u) == unsigned(-2));
static_assert(std::is_unsigned_v<decltype(min(-2, 0u, 7u))>);
static_assert(min(0u, 7u, -2) == unsigned(-2));
static_assert(min(7u, 0u, -2) == unsigned(-2));
static_assert(min(-2, -1, -7) == -7);
static_assert(std::is_signed_v<decltype(min(-2, -1, -7))>);
static_assert(min(std::numeric_limits<int>::min(), 0, 7) == std::numeric_limits<int>::min());
static_assert(min(std::numeric_limits<unsigned long long>::min(), 0, 7) ==
              std::numeric_limits<unsigned long long>::min());
static_assert(min(std::numeric_limits<signed long long>::min(), 0, 7) ==
              std::numeric_limits<signed long long>::min());
static_assert(min(std::numeric_limits<signed long long>::max(), 0, 7) == 0);
static_assert(min(std::numeric_limits<unsigned long long>::max(), 0, 7) == 0);
static_assert(min(std::numeric_limits<unsigned long long>::max(), 0, 7, std::numeric_limits<int>::min()) ==
              std::numeric_limits<int>::min());
static_assert(min(std::numeric_limits<unsigned long long>::max(),
                  0,
                  7,
                  std::numeric_limits<signed long int>::min()) == std::numeric_limits<signed long int>::min());
static_assert(min(std::numeric_limits<unsigned long long>::max(),
                  0,
                  7,
                  std::numeric_limits<signed long long>::min()) == std::numeric_limits<signed long long>::min());
static_assert(
    min(std::numeric_limits<unsigned long long>::max(), 0, 7, std::numeric_limits<signed long long>::max()) == 0);

struct foo
{
    constexpr foo(const int value)
      : value(value)
    {}
    constexpr foo(const foo& foo)
      : value(foo.value)
    {}
    constexpr foo(foo&& foo)
      : value(foo.value)
    {
        foo.value = -1;
    }
    int value;
    constexpr bool operator<(const foo& other) const { return value < other.value; }
    constexpr bool operator==(const foo& other) const { return value == other.value; }
};
struct bar : public foo
{
    constexpr bar(const int value)
      : foo(value)
    {}
    constexpr bar(const bar&) = default;
    constexpr bar(bar&&) = default;
    constexpr bool operator<(const bar& other) const { return value < other.value; }
    constexpr bool operator==(const bar& other) const { return value == other.value; }
};
inline constexpr bool operator<(const bar& a, const foo& b)
{
    return a.value < b.value;
}
struct string
{
    constexpr string(const char* s)
      : value(s)
    {}
    constexpr string(const string&) = default;
    constexpr string(string&&) = default;
    constexpr bool operator<(const string& other) const { return integer() < other.integer(); }
    constexpr bool operator==(const string& other) const { return integer() == other.integer(); }

    constexpr int integer() const
    {
        int v = 0;
        for (std::size_t i = 0; value[i] != '\0'; i++) {
            v *= 10;
            v += value[i] - '0';
        }
        return v;
    }

    const char* value;
};
inline constexpr bool operator<(int value, const string& other)
{
    return value < other;
}

static_assert(max(foo{ 2 }, foo{ 0 }, foo{ 7 }) == foo{ 7 });
inline constexpr auto ut1()
{
    std::size_t a = 42;
    std::size_t b = 100;
    auto&& v = max(a, b);
    static_assert(std::is_lvalue_reference_v<decltype(v)>);
    return ++v;
}
static_assert(ut1() == 101);

inline constexpr auto ut2()
{
    std::size_t a = 42;
    const int b = 100;
    return max(a, b);
}
static_assert(ut2() == 100);

inline constexpr auto ut3()
{
    std::size_t a = 42;
    std::size_t b = 100;
    std::size_t c = 1;
    std::size_t d = 3;
    std::size_t e = 45;
    std::size_t f = 78;
    auto&& v = max(a, b, c, d, e, f);
    static_assert(std::is_lvalue_reference_v<decltype(v)>);
    return ++v;
}
static_assert(ut3() == 101);

inline constexpr auto ut4()
{
    const std::size_t a = 42;
    const int b = -100;
    return max(a, b);
}
static_assert(max(int(-100), std::size_t(42), long(10)) == 42);
static_assert(max(std::size_t(42), int(-100)) == 42);
static_assert(ut4() == 42);

inline constexpr auto ut5()
{
    std::size_t a = 42;
    std::size_t b = 100;
    static_assert(std::is_rvalue_reference_v<decltype(std::move(max(std::move(a), std::move(b))))>);
    const std::size_t c{ max(std::move(a), std::move(b)) };
    return c;
}
static_assert(ut5() == 100);

inline constexpr auto ut6()
{
    foo a{ 42 };
    foo b{ 100 };
    static_assert(std::is_rvalue_reference_v<decltype(std::move(max(std::move(a), std::move(b))))>);
    const foo c{ std::move(max(a, b)) };
    return c.value + b.value;
}
static_assert(ut6() == 100 - 1);

inline constexpr auto ut7()
{
    foo a{ 42 };
    bar b{ 100 };
    const foo c{ std::move(max(a, b)) };
    return c.value + b.value;
}
static_assert(ut7() == 100 - 1);

static_assert(max(string("42"), string("100"), string("0"), string("39")).integer() == 100);

inline std::size_t ut8(std::size_t a = 42,
                       std::size_t b = 100,
                       std::size_t c = 1,
                       std::size_t d = 3,
                       std::size_t e = 45,
                       std::size_t f = 78)
{
    auto&& v = max(a, b, c, d, e, f);
    static_assert(std::is_lvalue_reference_v<decltype(v)>);
    return ++v;
}

} // namespace ut

} // namespace enhancedminmax
