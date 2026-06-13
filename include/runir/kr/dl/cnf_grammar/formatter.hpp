#ifndef RUNIR_CNF_GRAMMAR_FORMATTER_HPP_
#define RUNIR_CNF_GRAMMAR_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/formatter.hpp"
#include "runir/kr/dl/cnf_grammar/grammar_view.hpp"
#include "runir/kr/dl/cnf_grammar/views.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/formatting/cista_formatters.hpp>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir::kr::dl::cnf_grammar::format
{

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<typename Objects>
std::vector<std::string> quoted_object_names(Objects objects)
{
    auto result = std::vector<std::string> {};
    for (auto object : objects)
        result.push_back(fmt::format("{:?}", std::string(object.get_name().str())));
    return result;
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::string concept_constructor(ygg::View<ygg::Index<Concept<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return fmt::format("{}", runir::kr::dl::grammar::ast::ConceptBot<Family>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return fmt::format("{}", runir::kr::dl::grammar::ast::ConceptTop<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicState<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptIntersection<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptUnion<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNegation<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptValueRestriction<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptExistentialQuantification<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<Family>::keyword, view.get_n(), view.get_role());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<Family>::keyword,
                           view.get_n(),
                           view.get_role(),
                           view.get_concept());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptRoleValueMap<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptAgreement<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptRoleFillers<Family>::keyword,
                           view.get_role(),
                           fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptOneOf<Family>::keyword, fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::ConceptNominal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_object().get_name().str())));
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL concept constructor tag in concept_constructor");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
std::string role(ygg::View<ygg::Index<Role<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return fmt::format("{}", runir::kr::dl::grammar::ast::RoleUniversal<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicState<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleIntersection<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleUnion<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleComplement<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleInverse<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleComposition<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleTransitiveClosure<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleRestriction<Family>::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleIdentity<Family>::keyword, view.get_arg());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL role constructor tag in role");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
std::string boolean_constructor(ygg::View<ygg::Index<Boolean<Family, Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicState<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicGoal<Family>::keyword,
                           fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::BooleanNonempty<Family>::keyword, view.get_arg());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL boolean constructor tag in boolean_constructor");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
std::string numerical(ygg::View<ygg::Index<Numerical<Family, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::NumericalCount<Family>::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("{} {} {} {}", runir::kr::dl::grammar::ast::NumericalDistance<Family>::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL numerical constructor tag in numerical");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::string constructor_body(ygg::View<ygg::Index<Concept<Family, Tag>>, C> view)
{
    return concept_constructor(view);
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
std::string constructor_body(ygg::View<ygg::Index<Role<Family, Tag>>, C> view)
{
    return role(view);
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
std::string constructor_body(ygg::View<ygg::Index<Boolean<Family, Tag>>, C> view)
{
    return boolean_constructor(view);
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
std::string constructor_body(ygg::View<ygg::Index<Numerical<Family, Tag>>, C> view)
{
    return numerical(view);
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(ygg::View<ygg::Index<Constructor<Family, Category>>, C> view)
{
    return ygg::visit([](auto arg) { return constructor_body(arg); }, view.get_variant());
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string rule(ygg::View<ygg::Index<DerivationRule<Family, Category>>, C> view)
{
    return fmt::format("({} ({}))", view.get_lhs(), constructor_body(view.get_rhs()));
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string rule(ygg::View<ygg::Index<SubstitutionRule<Family, Category>>, C> view)
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
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Concept<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Concept<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::concept_constructor(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Role<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Role<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::role(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::boolean_constructor(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::numerical(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Constructor<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::cnf_grammar::format::constructor_body(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(view.get_name().str(), ctx); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::DerivationRule<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::rule(view), ctx); }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Family, Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::rule(view), ctx); }
};

template<runir::kr::dl::FamilyTag Family, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Grammar<Family>>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::dl::cnf_grammar::Grammar<Family>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::grammar(view), ctx);
    }
};
#endif

#endif
