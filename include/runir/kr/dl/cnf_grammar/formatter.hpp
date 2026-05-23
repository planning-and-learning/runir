#ifndef RUNIR_CNF_GRAMMAR_FORMATTER_HPP_
#define RUNIR_CNF_GRAMMAR_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/kr/dl/cnf_grammar/grammar_view.hpp"
#include "runir/kr/dl/cnf_grammar/views.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>
#include <tyr/common/formatter.hpp>
#include <vector>

namespace runir::kr::dl::cnf_grammar::format
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
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::string concept_constructor(tyr::View<tyr::Index<Concept<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return fmt::format("{}", runir::kr::dl::grammar::ast::ConceptBot::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return fmt::format("{}", runir::kr::dl::grammar::ast::ConceptTop::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNegation::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptValueRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptExistentialQuantification::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptExactNumberRestriction::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {} {}", runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return fmt::format("{} {} {} {}", runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return fmt::format("{} {} {} {}", runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction::keyword, view.get_n(), view.get_role(), view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptRoleValueMap::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptAgreement::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptRoleFillers::keyword, view.get_role(), fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptOneOf::keyword, fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNominal::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<Role<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return fmt::format("{}", runir::kr::dl::grammar::ast::RoleUniversal::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleComplement::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleInverse::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleComposition::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleIdentity::keyword, view.get_arg());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<Boolean<Family, Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicState::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::BooleanNonempty::keyword, view.get_arg());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<Numerical<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::NumericalCount::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("{} {} {} {}", runir::kr::dl::grammar::ast::NumericalDistance::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
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
    return view.get_variant().apply([](auto arg) { return constructor_body(arg); });
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string rule(tyr::View<tyr::Index<DerivationRule<Family, Category>>, C> view)
{
    return fmt::format("({} ({}))", view.get_lhs(), constructor_body(view.get_rhs()));
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string rule(tyr::View<tyr::Index<SubstitutionRule<Family, Category>>, C> view)
{
    return fmt::format("({} ({}))", view.get_lhs(), view.get_rhs());
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
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::ConceptTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::RoleTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::BooleanTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::NumericalTag>());
    text += ")\n";
    return text;
}

}  // namespace runir::kr::dl::cnf_grammar::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Concept<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Concept<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::concept_constructor(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Role<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Role<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::role(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::boolean_constructor(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::numerical(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::constructor_body(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(view.get_name().str(), ctx); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::rule(view), ctx); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::rule(view), ctx); }
};

template<runir::kr::dl::FamilyTag Family, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Grammar<Family>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Grammar<Family>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::grammar(view), ctx);
    }
};
#endif

#endif
