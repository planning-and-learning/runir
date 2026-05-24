#ifndef RUNIR_GRAMMAR_FORMATTER_HPP_
#define RUNIR_GRAMMAR_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/grammar_view.hpp"
#include "runir/kr/dl/grammar/views.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>
#include <tyr/common/formatter.hpp>
#include <vector>

namespace runir::kr::dl::grammar::format
{

template<typename String>
std::string quoted(const String& value)
{
    return fmt::format("\"{}\"", value.str());
}

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<typename Objects>
std::vector<std::string> quoted_object_names(Objects objects)
{
    auto result = std::vector<std::string> {};
    for (auto object : objects)
        result.push_back(quoted(object.get_name()));
    return result;
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
std::string concept_constructor(tyr::View<tyr::Index<Concept<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return fmt::format("{}", ast::ConceptBot<Family>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return fmt::format("{}", ast::ConceptTop<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", ast::ConceptAtomicState<Family>::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}", ast::ConceptAtomicGoal<Family>::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", ast::ConceptIntersection<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", ast::ConceptUnion<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("{} {}", ast::ConceptNegation<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("{} {} {}", ast::ConceptValueRestriction<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("{} {} {}", ast::ConceptExistentialQuantification<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {}", ast::ConceptAtLeastNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return fmt::format("{} {} {}", ast::ConceptAtMostNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return fmt::format("{} {} {}", ast::ConceptExactNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {} {}", ast::ConceptQualifiedAtLeastNumberRestriction<Family>::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return fmt::format("{} {} {} {}", ast::ConceptQualifiedAtMostNumberRestriction<Family>::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return fmt::format("{} {} {} {}", ast::ConceptQualifiedExactNumberRestriction<Family>::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return fmt::format("{} {} {}", ast::ConceptRoleValueMap<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return fmt::format("{} {} {}", ast::ConceptAgreement<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return fmt::format("{} {} {}", ast::ConceptRoleFillers<Family>::keyword, view.get_role(), fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return fmt::format("{} {}", ast::ConceptOneOf<Family>::keyword, fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("{} {}", ast::ConceptNominal<Family>::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<Role<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return fmt::format("{}", ast::RoleUniversal<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", ast::RoleAtomicState<Family>::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}", ast::RoleAtomicGoal<Family>::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", ast::RoleIntersection<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", ast::RoleUnion<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("{} {}", ast::RoleComplement<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("{} {}", ast::RoleInverse<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("{} {} {}", ast::RoleComposition<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("{} {}", ast::RoleTransitiveClosure<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("{} {}", ast::RoleReflexiveTransitiveClosure<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("{} {} {}", ast::RoleRestriction<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("{} {}", ast::RoleIdentity<Family>::keyword, view.get_arg());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<Boolean<Family, Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {} {}", ast::BooleanAtomicState<Family>::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}", ast::BooleanAtomicGoal<Family>::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("{} {}", ast::BooleanNonempty<Family>::keyword, view.get_arg());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<Numerical<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("{} {}", ast::NumericalCount<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("{} {} {} {}", ast::NumericalDistance<Family>::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
std::string constructor_body(tyr::View<tyr::Index<Concept<Family, Tag>>, C> view)
{
    return concept_constructor(view);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<Role<Family, Tag>>, C> view)
{
    return role(view);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<Boolean<Family, Tag>>, C> view)
{
    return boolean_constructor(view);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<Numerical<Family, Tag>>, C> view)
{
    return numerical(view);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(tyr::View<tyr::Index<Constructor<Family, Category>>, C> view)
{
    return tyr::visit([](auto arg) { return constructor_body(arg); }, view.get_variant());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string rule(tyr::View<tyr::Index<DerivationRule<Family, Category>>, C> view)
{
    auto rhs = std::vector<std::string> {};
    for (auto symbol : view.get_rhs())
    {
        rhs.push_back(tyr::visit(
            [](auto arg)
            {
                if constexpr (std::same_as<std::decay_t<decltype(arg)>, tyr::View<tyr::Index<Constructor<Family, Category>>, C>>)
                    return constructor_body(arg);
                else
                    return fmt::format("{}", arg);
            },
            symbol.get_variant()));
    }

    return fmt::format("({} ({}))", view.get_lhs(), fmt::join(rhs, " or "));
}

template<typename Rules>
void append_rules(std::string& text, Rules rules)
{
    for (auto rule : rules)
        text += fmt::format("    {}\n", rule);
}

template<typename View>
std::string grammar(View view)
{
    auto text = std::string("(\n");
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::ConceptTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::RoleTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::BooleanTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::NumericalTag>());
    text += ")\n";
    return text;
}

}  // namespace runir::kr::dl::grammar::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Concept<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Concept<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::concept_constructor(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Role<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Role<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::role(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Boolean<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Boolean<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::boolean_constructor(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Numerical<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Numerical<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::numerical(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Constructor<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Constructor<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::constructor_body(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::NonTerminal<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(view.get_name().str(), ctx); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(fmt::format("{}", view.get_variant()), ctx); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::DerivationRule<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::DerivationRule<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::grammar::format::rule(view), ctx); }
};

template<runir::kr::dl::FamilyTag Family, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::GrammarTag<Family>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::GrammarTag<Family>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::grammar::format::grammar(view), ctx); }
};
#endif

#endif
