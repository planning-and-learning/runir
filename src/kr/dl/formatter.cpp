#include "runir/kr/dl/formatter.hpp"

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/formatter.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/formatter.hpp"
#include "runir/kr/dl/record_view.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/formatter.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <urd/elements/concepts.hpp>
#include <urd/format.hpp>
#include <utility>
#include <yggdrasil/formatting/cista_formatters.hpp>

// A constructor record is written as its keyword and its identifying members (in record order; not a derived field
// such as a distance's plan), each after a space: a planning symbol (or a list of them) by its quoted name, a polarity as
// a Boolean keyword, anything else as fmt writes it (an operand as its record, a non-terminal by its name).

namespace dl = runir::kr::dl;
namespace dg = runir::kr::dl::grammar;
namespace dc = runir::kr::dl::cnf_grammar;

namespace
{

/// The keyword of a constructor record: its tag's, unless its category spells it (atomic constructors, the role
/// intersection and union, registers).
template<dl::CategoryTag Category, typename Tag>
constexpr std::string_view record_keyword = Tag::keyword;
template<tyr::formalism::FactKind T>
constexpr std::string_view record_keyword<dl::ConceptTag, dl::AtomicStateTag<T>> = dl::ConceptAtomicStateSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
constexpr std::string_view record_keyword<dl::ConceptTag, dl::AtomicGoalTag<T>> = dl::ConceptAtomicGoalSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
constexpr std::string_view record_keyword<dl::RoleTag, dl::AtomicStateTag<T>> = dl::RoleAtomicStateSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
constexpr std::string_view record_keyword<dl::RoleTag, dl::AtomicGoalTag<T>> = dl::RoleAtomicGoalSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
constexpr std::string_view record_keyword<dl::BooleanTag, dl::AtomicStateTag<T>> = dl::BooleanAtomicStateSyntaxTag::keyword;
template<tyr::formalism::FactKind T>
constexpr std::string_view record_keyword<dl::BooleanTag, dl::AtomicGoalTag<T>> = dl::BooleanAtomicGoalSyntaxTag::keyword;
template<>
constexpr std::string_view record_keyword<dl::RoleTag, dl::IntersectionTag> = dl::RoleIntersectionSyntaxTag::keyword;
template<>
constexpr std::string_view record_keyword<dl::RoleTag, dl::UnionTag> = dl::RoleUnionSyntaxTag::keyword;
template<>
constexpr std::string_view record_keyword<dl::ConceptTag, dl::RegisterTag> = dl::ConceptRegisterSyntaxTag::keyword;
template<>
constexpr std::string_view record_keyword<dl::RoleTag, dl::RegisterTag> = dl::RoleRegisterSyntaxTag::keyword;

/// Whether X is a planning symbol (a predicate or an object view), which a record writes as its quoted name.
template<typename X>
constexpr bool is_planning_symbol = false;
template<tyr::formalism::FactKind K, typename C>
constexpr bool is_planning_symbol<urd::index_view<tyr::formalism::Predicate<K>, C>> = true;
template<typename C>
constexpr bool is_planning_symbol<urd::index_view<tyr::formalism::Object, C>> = true;

/// A list of planning symbols (objects).
template<typename T>
concept SymbolList = std::ranges::range<T> && is_planning_symbol<std::remove_cvref_t<std::ranges::range_value_t<T>>>;

}  // namespace

template<typename F, typename Tag, typename C>
fmt::format_context::iterator fmt::formatter<std::pair<urd::index_view<dl::Concept<F, Tag>, C>, dl::Unparenthesized>>::format(
    const std::pair<urd::index_view<dl::Concept<F, Tag>, C>, dl::Unparenthesized>& value,
    fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "{}", record_keyword<dl::ConceptTag, Tag>);
    const auto write = [&out]<typename T>(const T& component)
    {
        if constexpr (is_planning_symbol<T>)
            out = fmt::format_to(out, " {:?}", component.get_name().view());
        else if constexpr (std::same_as<T, bool>)
            out = fmt::format_to(out, " {}", component ? dl::TrueTag::keyword : dl::FalseTag::keyword);
        else if constexpr (SymbolList<T>)
            for (auto symbol : component)
                out = fmt::format_to(out, " {:?}", symbol.get_name().view());
        else
            out = fmt::format_to(out, " {}", component);
    };
    [&]<auto... Ms>(urd::Members<Ms...> /*members*/)
    { (write(value.first.template member<Ms>()), ...); }(urd::record_of<urd::index_view<dl::Concept<F, Tag>, C>>::identifying_members());
    return out;
}

template<typename F, typename Tag, typename C>
fmt::format_context::iterator fmt::formatter<urd::index_view<dl::Concept<F, Tag>, C>>::format(const urd::index_view<dl::Concept<F, Tag>, C>& value,
                                                                                              fmt::format_context& context) const
{ return fmt::format_to(context.out(), "({})", std::pair { value, dl::Unparenthesized {} }); }

template<typename F, typename Tag, typename C>
fmt::format_context::iterator fmt::formatter<std::pair<urd::index_view<dl::Role<F, Tag>, C>, dl::Unparenthesized>>::format(
    const std::pair<urd::index_view<dl::Role<F, Tag>, C>, dl::Unparenthesized>& value,
    fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "{}", record_keyword<dl::RoleTag, Tag>);
    const auto write = [&out]<typename T>(const T& component)
    {
        if constexpr (is_planning_symbol<T>)
            out = fmt::format_to(out, " {:?}", component.get_name().view());
        else if constexpr (std::same_as<T, bool>)
            out = fmt::format_to(out, " {}", component ? dl::TrueTag::keyword : dl::FalseTag::keyword);
        else if constexpr (SymbolList<T>)
            for (auto symbol : component)
                out = fmt::format_to(out, " {:?}", symbol.get_name().view());
        else
            out = fmt::format_to(out, " {}", component);
    };
    [&]<auto... Ms>(urd::Members<Ms...> /*members*/)
    { (write(value.first.template member<Ms>()), ...); }(urd::record_of<urd::index_view<dl::Role<F, Tag>, C>>::identifying_members());
    return out;
}

template<typename F, typename Tag, typename C>
fmt::format_context::iterator fmt::formatter<urd::index_view<dl::Role<F, Tag>, C>>::format(const urd::index_view<dl::Role<F, Tag>, C>& value,
                                                                                           fmt::format_context& context) const
{ return fmt::format_to(context.out(), "({})", std::pair { value, dl::Unparenthesized {} }); }

template<typename F, typename Tag, typename C>
fmt::format_context::iterator fmt::formatter<std::pair<urd::index_view<dl::Boolean<F, Tag>, C>, dl::Unparenthesized>>::format(
    const std::pair<urd::index_view<dl::Boolean<F, Tag>, C>, dl::Unparenthesized>& value,
    fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "{}", record_keyword<dl::BooleanTag, Tag>);
    const auto write = [&out]<typename T>(const T& component)
    {
        if constexpr (is_planning_symbol<T>)
            out = fmt::format_to(out, " {:?}", component.get_name().view());
        else if constexpr (std::same_as<T, bool>)
            out = fmt::format_to(out, " {}", component ? dl::TrueTag::keyword : dl::FalseTag::keyword);
        else if constexpr (SymbolList<T>)
            for (auto symbol : component)
                out = fmt::format_to(out, " {:?}", symbol.get_name().view());
        else
            out = fmt::format_to(out, " {}", component);
    };
    [&]<auto... Ms>(urd::Members<Ms...> /*members*/)
    { (write(value.first.template member<Ms>()), ...); }(urd::record_of<urd::index_view<dl::Boolean<F, Tag>, C>>::identifying_members());
    return out;
}

template<typename F, typename Tag, typename C>
fmt::format_context::iterator fmt::formatter<urd::index_view<dl::Boolean<F, Tag>, C>>::format(const urd::index_view<dl::Boolean<F, Tag>, C>& value,
                                                                                              fmt::format_context& context) const
{ return fmt::format_to(context.out(), "({})", std::pair { value, dl::Unparenthesized {} }); }

template<typename F, typename Tag, typename C>
fmt::format_context::iterator fmt::formatter<std::pair<urd::index_view<dl::Numerical<F, Tag>, C>, dl::Unparenthesized>>::format(
    const std::pair<urd::index_view<dl::Numerical<F, Tag>, C>, dl::Unparenthesized>& value,
    fmt::format_context& context) const
{
    auto out = fmt::format_to(context.out(), "{}", record_keyword<dl::NumericalTag, Tag>);
    const auto write = [&out]<typename T>(const T& component)
    {
        if constexpr (is_planning_symbol<T>)
            out = fmt::format_to(out, " {:?}", component.get_name().view());
        else if constexpr (std::same_as<T, bool>)
            out = fmt::format_to(out, " {}", component ? dl::TrueTag::keyword : dl::FalseTag::keyword);
        else if constexpr (SymbolList<T>)
            for (auto symbol : component)
                out = fmt::format_to(out, " {:?}", symbol.get_name().view());
        else
            out = fmt::format_to(out, " {}", component);
    };
    [&]<auto... Ms>(urd::Members<Ms...> /*members*/)
    { (write(value.first.template member<Ms>()), ...); }(urd::record_of<urd::index_view<dl::Numerical<F, Tag>, C>>::identifying_members());
    return out;
}

template<typename F, typename Tag, typename C>
fmt::format_context::iterator fmt::formatter<urd::index_view<dl::Numerical<F, Tag>, C>>::format(const urd::index_view<dl::Numerical<F, Tag>, C>& value,
                                                                                                fmt::format_context& context) const
{ return fmt::format_to(context.out(), "({})", std::pair { value, dl::Unparenthesized {} }); }

// Each family's constructor tags (F::Tags<Category>: SemanticFamily every constructor tag, the grammar families the base
// constructor tags), each through the repository its family's records are viewed through.

// dl::SemanticFamily through dl::ConstructorRepository.
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::BotTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::TopTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::IntersectionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::UnionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::NegationTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::ValueRestrictionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::ExistentialQuantificationTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AtLeastNumberRestrictionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AtMostNumberRestrictionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::ExactNumberRestrictionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::QualifiedAtLeastNumberRestrictionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::QualifiedAtMostNumberRestrictionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::QualifiedExactNumberRestrictionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::RoleValueMapTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::AgreementTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::RoleFillersTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::OneOfTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::NominalTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::RegisterTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dl::SemanticFamily, dl::ArgumentTag<dl::ConceptTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::UniversalTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::IntersectionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::UnionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::ComplementTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::InverseTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::CompositionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::TransitiveClosureTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::ReflexiveTransitiveClosureTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::RestrictionTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::IdentityTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::RegisterTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dl::SemanticFamily, dl::ArgumentTag<dl::RoleTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::NonemptyTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::BooleanEqTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::BooleanNeTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::BooleanLtTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::BooleanLeTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::BooleanGtTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::BooleanGeTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::NumericalEqTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::NumericalNeTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::NumericalLtTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::NumericalLeTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::NumericalGtTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::NumericalGeTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::BooleanConstantTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::AndTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::OrTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::NotTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dl::SemanticFamily, dl::ArgumentTag<dl::BooleanTag>>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::CountTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::DistanceTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::NumericalConstantTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::AddTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::SubTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::MulTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::DivTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::MinTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::MaxTag>, dl::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dl::SemanticFamily, dl::ArgumentTag<dl::NumericalTag>>, dl::ConstructorRepository>>;

// dg::GrammarFamily through dg::ConstructorRepository.
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::BotTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::TopTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::IntersectionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::UnionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::NegationTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::ValueRestrictionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::ExistentialQuantificationTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtLeastNumberRestrictionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtMostNumberRestrictionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::ExactNumberRestrictionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::QualifiedAtLeastNumberRestrictionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::QualifiedAtMostNumberRestrictionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::QualifiedExactNumberRestrictionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::RoleValueMapTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AgreementTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::RoleFillersTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::OneOfTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dg::GrammarFamily, dl::NominalTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::UniversalTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::IntersectionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::UnionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::ComplementTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::InverseTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::CompositionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::TransitiveClosureTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::ReflexiveTransitiveClosureTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::RestrictionTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dg::GrammarFamily, dl::IdentityTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::NonemptyTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dg::GrammarFamily, dl::CountTag>, dg::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dg::GrammarFamily, dl::DistanceTag>, dg::ConstructorRepository>>;

// dc::CnfGrammarFamily through dc::ConstructorRepository.
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::BotTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::TopTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::IntersectionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::UnionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::NegationTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::ValueRestrictionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::ExistentialQuantificationTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtLeastNumberRestrictionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtMostNumberRestrictionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::ExactNumberRestrictionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::QualifiedAtLeastNumberRestrictionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::QualifiedAtMostNumberRestrictionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::QualifiedExactNumberRestrictionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::RoleValueMapTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AgreementTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::RoleFillersTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::OneOfTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::NominalTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::UniversalTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::IntersectionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::UnionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::ComplementTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::InverseTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::CompositionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::TransitiveClosureTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::ReflexiveTransitiveClosureTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::RestrictionTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::IdentityTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::NonemptyTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dc::CnfGrammarFamily, dl::CountTag>, dc::ConstructorRepository>>;
template struct fmt::formatter<urd::index_view<dl::Numerical<dc::CnfGrammarFamily, dl::DistanceTag>, dc::ConstructorRepository>>;

// A grammar rule's alternatives (grammar and cnf_grammar families).
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::BotTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::TopTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::IntersectionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::UnionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::NegationTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::ValueRestrictionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::ExistentialQuantificationTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtLeastNumberRestrictionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AtMostNumberRestrictionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::ExactNumberRestrictionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::QualifiedAtLeastNumberRestrictionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::QualifiedAtMostNumberRestrictionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::QualifiedExactNumberRestrictionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::RoleValueMapTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::AgreementTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::RoleFillersTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::OneOfTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dg::GrammarFamily, dl::NominalTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::UniversalTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::IntersectionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::UnionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::ComplementTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::InverseTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::CompositionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::TransitiveClosureTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::ReflexiveTransitiveClosureTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::RestrictionTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dg::GrammarFamily, dl::IdentityTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Boolean<dg::GrammarFamily, dl::NonemptyTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Numerical<dg::GrammarFamily, dl::CountTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Numerical<dg::GrammarFamily, dl::DistanceTag>, dg::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::BotTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::TopTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::IntersectionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::UnionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::NegationTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::ValueRestrictionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::ExistentialQuantificationTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtLeastNumberRestrictionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AtMostNumberRestrictionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::ExactNumberRestrictionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::QualifiedAtLeastNumberRestrictionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::QualifiedAtMostNumberRestrictionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::QualifiedExactNumberRestrictionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::RoleValueMapTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::AgreementTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::RoleFillersTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::OneOfTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Concept<dc::CnfGrammarFamily, dl::NominalTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::UniversalTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::IntersectionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::UnionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::ComplementTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::InverseTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::CompositionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::TransitiveClosureTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::ReflexiveTransitiveClosureTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::RestrictionTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Role<dc::CnfGrammarFamily, dl::IdentityTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicStateTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::StaticTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::FluentTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::AtomicGoalTag<tyr::formalism::DerivedTag>>, dc::ConstructorRepository>,
              dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Boolean<dc::CnfGrammarFamily, dl::NonemptyTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<std::pair<urd::index_view<dl::Numerical<dc::CnfGrammarFamily, dl::CountTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
template struct fmt::formatter<
    std::pair<urd::index_view<dl::Numerical<dc::CnfGrammarFamily, dl::DistanceTag>, dc::ConstructorRepository>, dl::Unparenthesized>>;
