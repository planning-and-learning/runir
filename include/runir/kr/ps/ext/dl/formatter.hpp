#ifndef RUNIR_KR_PS_EXT_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_EXT_DL_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/formatter.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser/ext/ast.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/dl/formatter.hpp"
#include "runir/kr/ps/ext/dl/structural_termination_data.hpp"

#include <concepts>
#include <fmt/format.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formatting/dynamic_bitset_formatters.hpp>

namespace runir::kr::ps::ext::dl::format
{

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<typename... Components>
std::string constructor_expression(std::string_view keyword, Components&&... components)
{
    auto os = std::ostringstream {};
    os << '(' << keyword;
    ((os << ' ' << fmt::format("{}", std::forward<Components>(components))), ...);
    os << ')';
    return os.str();
}

template<typename Objects>
std::string constructor_expression_objects(std::string_view keyword, Objects objects)
{
    auto os = std::ostringstream {};
    os << '(' << keyword;
    for (auto object : objects)
        os << ' ' << fmt::format("{:?}", std::string(object.get_name().str()));
    os << ')';
    return os.str();
}

template<typename Head, typename Objects>
std::string constructor_expression_objects(std::string_view keyword, Head&& head, Objects objects)
{
    auto os = std::ostringstream {};
    os << '(' << keyword;
    os << ' ' << fmt::format("{}", std::forward<Head>(head));
    for (auto object : objects)
        os << ' ' << fmt::format("{:?}", std::string(object.get_name().str()));
    os << ')';
    return os.str();
}

template<typename T>
std::string expression(T value);

template<typename Variant>
std::string variant_expression(const Variant& value)
{
    return ygg::visit([](auto child) { return expression(child); }, value);
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
std::string concept_expression(ygg::View<ygg::Index<runir::kr::dl::FamilyConcept<Family, Tag>>, C> view)
{
    namespace ast = runir::kr::dl::grammar::ast;
    namespace ext_ast = runir::kr::dl::grammar::parser::ext::ast;

    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return constructor_expression(ast::ConceptBot<Family>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return constructor_expression(ast::ConceptTop<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor_expression(ast::ConceptAtomicState<Family>::keyword, fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor_expression(ast::ConceptAtomicGoal<Family>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                                      boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return constructor_expression(ast::ConceptIntersection<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return constructor_expression(ast::ConceptUnion<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return constructor_expression(ast::ConceptNegation<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return constructor_expression(ast::ConceptValueRestriction<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return constructor_expression(ast::ConceptExistentialQuantification<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return constructor_expression(ast::ConceptAtLeastNumberRestriction<Family>::keyword, view.get_n(), expression(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return constructor_expression(ast::ConceptAtMostNumberRestriction<Family>::keyword, view.get_n(), expression(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return constructor_expression(ast::ConceptExactNumberRestriction<Family>::keyword, view.get_n(), expression(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return constructor_expression(ast::ConceptQualifiedAtLeastNumberRestriction<Family>::keyword,
                                      view.get_n(),
                                      expression(view.get_role()),
                                      expression(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return constructor_expression(ast::ConceptQualifiedAtMostNumberRestriction<Family>::keyword,
                                      view.get_n(),
                                      expression(view.get_role()),
                                      expression(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return constructor_expression(ast::ConceptQualifiedExactNumberRestriction<Family>::keyword,
                                      view.get_n(),
                                      expression(view.get_role()),
                                      expression(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return constructor_expression(ast::ConceptRoleValueMap<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return constructor_expression(ast::ConceptAgreement<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return constructor_expression_objects(ast::ConceptRoleFillers<Family>::keyword, expression(view.get_role()), view.get_objects());
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return constructor_expression_objects(ast::ConceptOneOf<Family>::keyword, view.get_objects());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return constructor_expression(ast::ConceptNominal<Family>::keyword, fmt::format("{:?}", std::string(view.get_object().get_name().str())));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return constructor_expression(ext_ast::ConceptRegister::keyword, ygg::uint_t(view.get_data().identifier));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>)
        return constructor_expression(ext_ast::Argument<runir::kr::dl::ConceptTag>::keyword, ygg::uint_t(view.get_data().identifier));
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL concept constructor tag in concept_expression");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
std::string role(ygg::View<ygg::Index<runir::kr::dl::FamilyRole<Family, Tag>>, C> view)
{
    namespace ast = runir::kr::dl::grammar::ast;
    namespace ext_ast = runir::kr::dl::grammar::parser::ext::ast;

    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return constructor_expression(ast::RoleUniversal<Family>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor_expression(ast::RoleAtomicState<Family>::keyword, fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor_expression(ast::RoleAtomicGoal<Family>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                                      boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return constructor_expression(ast::RoleIntersection<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return constructor_expression(ast::RoleUnion<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return constructor_expression(ast::RoleComplement<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return constructor_expression(ast::RoleInverse<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return constructor_expression(ast::RoleComposition<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return constructor_expression(ast::RoleTransitiveClosure<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return constructor_expression(ast::RoleReflexiveTransitiveClosure<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return constructor_expression(ast::RoleRestriction<Family>::keyword, expression(view.get_lhs()), expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return constructor_expression(ast::RoleIdentity<Family>::keyword, expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RegisterTag>)
        return constructor_expression(ext_ast::RoleRegister::keyword, ygg::uint_t(view.get_data().identifier));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>)
        return constructor_expression(ext_ast::Argument<runir::kr::dl::RoleTag>::keyword, ygg::uint_t(view.get_data().identifier));
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL role constructor tag in role");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
std::string boolean_constructor(ygg::View<ygg::Index<runir::kr::dl::FamilyBoolean<Family, Tag>>, C> view)
{
    namespace ast = runir::kr::dl::grammar::ast;
    namespace ext_ast = runir::kr::dl::grammar::parser::ext::ast;

    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor_expression(ast::BooleanAtomicState<Family>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                                      boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor_expression(ast::BooleanAtomicGoal<Family>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                                      boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return constructor_expression(ast::BooleanNonempty<Family>::keyword, variant_expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>)
        return constructor_expression(ext_ast::Argument<runir::kr::dl::BooleanTag>::keyword, ygg::uint_t(view.get_data().identifier));
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL boolean constructor tag in boolean_constructor");
    }
}

template<runir::kr::dl::FamilyTag Family, typename Tag, typename C>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
std::string numerical(ygg::View<ygg::Index<runir::kr::dl::FamilyNumerical<Family, Tag>>, C> view)
{
    namespace ast = runir::kr::dl::grammar::ast;
    namespace ext_ast = runir::kr::dl::grammar::parser::ext::ast;

    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return constructor_expression(ast::NumericalCount<Family>::keyword, variant_expression(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return constructor_expression(ast::NumericalDistance<Family>::keyword,
                                      expression(view.get_lhs()),
                                      expression(view.get_mid()),
                                      expression(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>)
        return constructor_expression(ext_ast::Argument<runir::kr::dl::NumericalTag>::keyword, ygg::uint_t(view.get_data().identifier));
    else
    {
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL numerical constructor tag in numerical");
    }
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(ygg::View<ygg::Index<runir::kr::dl::FamilyConstructor<Family, Category>>, C> view)
{
    return variant_expression(view.get_variant());
}

template<typename T>
std::string expression(T value)
{
    if constexpr (requires { concept_expression(value); })
        return concept_expression(value);
    else if constexpr (requires { role(value); })
        return role(value);
    else if constexpr (requires { boolean_constructor(value); })
        return boolean_constructor(value);
    else if constexpr (requires { numerical(value); })
        return numerical(value);
    else if constexpr (requires { constructor(value); })
        return constructor(value);
    else
        return variant_expression(value);
}

}  // namespace runir::kr::ps::ext::dl::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModulePolicyGraphVertexLabel, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::ext::dl::ModulePolicyGraphVertexLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("(concepts={}, booleans={}, numericals={}, memory={})",
                                      label.concept_values,
                                      label.boolean_values,
                                      label.numerical_values,
                                      label.memory_state.get_name().str());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModulePolicyGraphEdgeLabel, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::ext::dl::ModulePolicyGraphEdgeLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("(rule={}, numerical_changes={})", label.rule.get_index().get_value(), label.numerical_changes);
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct fmt::formatter<runir::kr::ps::ext::dl::ModuleStructuralTerminationResult, char> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::ext::dl::ModuleStructuralTerminationResult& result, FormatContext& ctx) const
    {
        const auto text = result.is_terminating() ?
                              std::string { "ModuleStructuralTerminationResult(terminating)" } :
                              fmt::format("ModuleStructuralTerminationResult(non-terminating, counterexample with {} vertices and {} edges)",
                                          result.counterexample->get_num_vertices(),
                                          result.counterexample->get_num_edges());
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};
#endif

#endif
