#ifndef RUNIR_KR_DL_FORMATTER_HPP_
#define RUNIR_KR_DL_FORMATTER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/record_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <urd/elements/concepts.hpp>
#include <urd/elements/view.hpp>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir::kr::dl
{

inline std::string boolean(bool value) { return value ? TrueTag::keyword : FalseTag::keyword; }

/// The keyword of a constructor record: its tag's, unless its category spells it (atomic constructors, the role
/// intersection and union, registers).
template<CategoryTag Category, typename Tag>
inline constexpr std::string_view record_keyword = Tag::keyword;
template<tyr::formalism::FactKind T>
inline constexpr std::string_view record_keyword<ConceptTag, AtomicStateTag<T>> = ConceptAtomicStateSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
inline constexpr std::string_view record_keyword<ConceptTag, AtomicGoalTag<T>> = ConceptAtomicGoalSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
inline constexpr std::string_view record_keyword<RoleTag, AtomicStateTag<T>> = RoleAtomicStateSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
inline constexpr std::string_view record_keyword<RoleTag, AtomicGoalTag<T>> = RoleAtomicGoalSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
inline constexpr std::string_view record_keyword<BooleanTag, AtomicStateTag<T>> = BooleanAtomicStateSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
inline constexpr std::string_view record_keyword<BooleanTag, AtomicGoalTag<T>> = BooleanAtomicGoalSyntaxTag::keyword;
template<>
inline constexpr std::string_view record_keyword<RoleTag, IntersectionTag> = RoleIntersectionSyntaxTag::keyword;
template<>
inline constexpr std::string_view record_keyword<RoleTag, UnionTag> = RoleUnionSyntaxTag::keyword;
template<>
inline constexpr std::string_view record_keyword<ConceptTag, RegisterTag> = ConceptRegisterSyntaxTag::keyword;
template<>
inline constexpr std::string_view record_keyword<RoleTag, RegisterTag> = RoleRegisterSyntaxTag::keyword;

/// Whether X is a planning symbol (a predicate or an object view), which a record prints as its quoted name.
template<typename X>
inline constexpr bool is_planning_symbol = false;
template<tyr::formalism::FactKind K, typename C>
inline constexpr bool is_planning_symbol<urd::index_view<tyr::formalism::Predicate<K>, C>> = true;
template<typename C>
inline constexpr bool is_planning_symbol<urd::index_view<tyr::formalism::Object, C>> = true;
/// A list of planning symbols (objects).
template<typename T>
concept SymbolList = std::ranges::range<T> && is_planning_symbol<std::remove_cvref_t<std::ranges::range_value_t<T>>>;

/// The keyword and components of a constructor record, without parentheses: its fields, in record order (urd::Fields),
/// each as it is written: a planning symbol (or a list of them) by its quoted name, a polarity as a Boolean keyword,
/// anything else as fmt formats it.
template<CategoryTag Category, typename Tag, typename View>
std::string record_text(View view)
{
    auto text = std::string(record_keyword<Category, Tag>);
    const auto add = [&text](const auto& component) { text += fmt::format(" {}", component); };
    const auto add_name = [&text](auto symbol) { text += fmt::format(" {:?}", symbol.get_name().view()); };
    const auto add_component = [&]<typename T>(const T& component)
    {
        if constexpr (is_planning_symbol<T>)
            add_name(component);
        else if constexpr (std::same_as<T, bool>)
            add(boolean(component));
        else if constexpr (SymbolList<T>)
            for (auto symbol : component)
                add_name(symbol);
        else
            add(component);
    };
    urd::for_each_field<urd::record_of<View>>([&]<typename F>() { add_component(view.template member<F::member>()); });
    return text;
}

template<typename F, typename Tag, typename C>
std::string record_body(urd::index_view<Concept<F, Tag>, C> view)
{ return record_text<ConceptTag, Tag>(view); }

template<typename F, typename Tag, typename C>
std::string record_body(urd::index_view<Role<F, Tag>, C> view)
{ return record_text<RoleTag, Tag>(view); }

template<typename F, typename Tag, typename C>
std::string record_body(urd::index_view<Boolean<F, Tag>, C> view)
{ return record_text<BooleanTag, Tag>(view); }

template<typename F, typename Tag, typename C>
std::string record_body(urd::index_view<Numerical<F, Tag>, C> view)
{ return record_text<NumericalTag, Tag>(view); }

}  // namespace runir::kr::dl

template<typename F, typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Concept<F, Tag>, C>> : fmt::formatter<std::string_view>
{
    auto format(urd::index_view<runir::kr::dl::Concept<F, Tag>, C> view, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(fmt::format("({})", runir::kr::dl::record_body(view)), ctx); }
};

template<typename F, typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Role<F, Tag>, C>> : fmt::formatter<std::string_view>
{
    auto format(urd::index_view<runir::kr::dl::Role<F, Tag>, C> view, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(fmt::format("({})", runir::kr::dl::record_body(view)), ctx); }
};

template<typename F, typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Boolean<F, Tag>, C>> : fmt::formatter<std::string_view>
{
    auto format(urd::index_view<runir::kr::dl::Boolean<F, Tag>, C> view, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(fmt::format("({})", runir::kr::dl::record_body(view)), ctx); }
};

template<typename F, typename Tag, typename C>
struct fmt::formatter<urd::index_view<runir::kr::dl::Numerical<F, Tag>, C>> : fmt::formatter<std::string_view>
{
    auto format(urd::index_view<runir::kr::dl::Numerical<F, Tag>, C> view, format_context& ctx) const
    { return fmt::formatter<std::string_view>::format(fmt::format("({})", runir::kr::dl::record_body(view)), ctx); }
};

#endif
