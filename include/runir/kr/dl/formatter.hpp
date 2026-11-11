#ifndef RUNIR_KR_DL_FORMATTER_HPP_
#define RUNIR_KR_DL_FORMATTER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/record_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <string>
#include <string_view>
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

/// The keyword and components of a constructor record, without parentheses: what its view holds, in record order.
/// Concept and role atoms of the state print no polarity.
template<CategoryTag Category, typename Tag, typename View>
std::string record_text(View view)
{
    auto text = std::string(record_keyword<Category, Tag>);
    const auto add = [&text](const auto& component) { text += fmt::format(" {}", component); };
    const auto add_name = [&text](auto symbol) { text += fmt::format(" {:?}", symbol.get_name().view()); };

    if constexpr (requires { view.get_predicate(); })
        add_name(view.get_predicate());
    if constexpr (requires { view.get_polarity(); } && !(is_atomic_state_tag_v<Tag> && !std::same_as<Category, BooleanTag>) )
        add(boolean(view.get_polarity()));
    if constexpr (requires { view.get_n(); })
        add(view.get_n());
    if constexpr (requires { view.get_role(); })
        add(view.get_role());
    if constexpr (requires { view.get_concept(); })
        add(view.get_concept());
    if constexpr (requires { view.get_lhs(); })
        add(view.get_lhs());
    if constexpr (requires { view.get_mid(); })
        add(view.get_mid());
    if constexpr (requires { view.get_rhs(); })
        add(view.get_rhs());
    if constexpr (requires { view.get_arg(); })
        add(view.get_arg());
    if constexpr (requires { view.get_register(); })
        add(view.get_register());
    if constexpr (requires { view.get_argument(); })
        add(view.get_argument());
    if constexpr (requires { view.get_value(); })
        add(fmt::format("{}", view.get_value()));
    if constexpr (requires { view.get_object(); })
        add_name(view.get_object());
    if constexpr (requires { view.get_objects(); })
        for (auto object : view.get_objects())
            add_name(object);
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
