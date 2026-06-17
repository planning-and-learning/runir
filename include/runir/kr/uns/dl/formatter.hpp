#ifndef RUNIR_KR_UNS_DL_FORMATTER_HPP_
#define RUNIR_KR_UNS_DL_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/dl/uns/declarations.hpp"
#include "runir/kr/uns/dl/feature_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::uns::dl::format
{

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

inline void append_component(std::ostream& os, std::string_view component)
{
    auto stream = std::istringstream(std::string(component));
    auto line = std::string {};
    auto first = true;
    while (std::getline(stream, line))
    {
        if (!first)
            os << '\n';
        os << ygg::print_indent << line;
        first = false;
    }
}

template<typename... Components>
std::string constructor_components(std::string_view keyword, Components&&... components)
{
    auto os = std::ostringstream {};
    os << keyword;
    {
        ygg::IndentScope scope(os);
        ((os << '\n', append_component(os, fmt::format("{}", std::forward<Components>(components)))), ...);
    }
    return os.str();
}

template<typename Objects>
std::string constructor_objects(std::string_view keyword, Objects objects)
{
    auto os = std::ostringstream {};
    os << keyword;
    {
        ygg::IndentScope scope(os);
        for (auto object : objects)
            os << '\n' << ygg::print_indent << fmt::format("{:?}", std::string(object.get_name().str()));
    }
    return os.str();
}

template<typename Head, typename Objects>
std::string constructor_objects(std::string_view keyword, Head&& head, Objects objects)
{
    auto os = std::ostringstream {};
    os << keyword;
    {
        ygg::IndentScope scope(os);
        os << '\n';
        append_component(os, fmt::format("{}", std::forward<Head>(head)));
        for (auto object : objects)
            os << '\n' << ygg::print_indent << fmt::format("{:?}", std::string(object.get_name().str()));
    }
    return os.str();
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(ygg::View<ygg::Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, Category>>, C> view);

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, Category>>, C> view);

template<typename View>
std::string constructor_variant(View view)
{
    return ygg::visit([](auto arg) { return constructor(arg); }, view);
}

template<runir::kr::dl::BaseConceptConstructorTag Tag, typename C>
std::string concept_constructor(ygg::View<ygg::Index<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return std::string(runir::kr::dl::grammar::ast::ConceptBot<runir::kr::UnsFamilyTag>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return std::string(runir::kr::dl::grammar::ast::ConceptTop<runir::kr::UnsFamilyTag>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::UnsFamilyTag>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::UnsFamilyTag>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                                      boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::UnsFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::UnsFamilyTag>::keyword,
                                      view.get_n(),
                                      constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::UnsFamilyTag>::keyword,
                                      view.get_n(),
                                      constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::UnsFamilyTag>::keyword,
                                      view.get_n(),
                                      constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::UnsFamilyTag>::keyword,
                                      view.get_n(),
                                      constructor(view.get_role()),
                                      constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::UnsFamilyTag>::keyword,
                                      view.get_n(),
                                      constructor(view.get_role()),
                                      constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::UnsFamilyTag>::keyword,
                                      view.get_n(),
                                      constructor(view.get_role()),
                                      constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return constructor_objects(runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::UnsFamilyTag>::keyword,
                                   constructor(view.get_role()),
                                   view.get_objects());
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return constructor_objects(runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::UnsFamilyTag>::keyword, view.get_objects());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return constructor_components(runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::UnsFamilyTag>::keyword,
                                      fmt::format("{:?}", std::string(view.get_object().get_name().str())));
    else
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL concept constructor tag in concept_constructor");
}

template<runir::kr::dl::BaseRoleConstructorTag Tag, typename C>
std::string role(ygg::View<ygg::Index<runir::kr::dl::Role<runir::kr::UnsFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return std::string(runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::UnsFamilyTag>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::UnsFamilyTag>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::UnsFamilyTag>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                                      boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleUnion<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleComplement<runir::kr::UnsFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleInverse<runir::kr::UnsFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleComposition<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::UnsFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return constructor_components(runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::UnsFamilyTag>::keyword, constructor(view.get_arg()));
    else
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL role constructor tag in role");
}

template<runir::kr::dl::UnsBooleanConstructorTag Tag, typename C>
std::string boolean_constructor(ygg::View<ygg::Index<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return constructor_components(runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::UnsFamilyTag>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                                      boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return constructor_components(runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::UnsFamilyTag>::keyword,
                                      fmt::format("{:?}", std::string(view.get_predicate().get_name().str())),
                                      boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return constructor_components(runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::UnsFamilyTag>::keyword, constructor_variant(view.get_arg()));
    else if constexpr (runir::kr::dl::ComparisonTag<Tag>)
        return constructor_components(runir::kr::dl::comparison_keyword<Tag>(), constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (runir::kr::dl::LogicalBinaryTag<Tag>)
        return constructor_components(runir::kr::dl::logical_binary_keyword<Tag>(), constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NotTag>)
        return constructor_components(runir::kr::dl::logical_not_keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::BooleanConstantTag>)
        return constructor_components(runir::kr::dl::boolean_constant_keyword, boolean(view.get_value()));
    else
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL boolean constructor tag in boolean_constructor");
}

template<runir::kr::dl::UnsNumericalConstructorTag Tag, typename C>
std::string numerical(ygg::View<ygg::Index<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return constructor_components(runir::kr::dl::grammar::ast::NumericalCount<runir::kr::UnsFamilyTag>::keyword, constructor_variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return constructor_components(runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::UnsFamilyTag>::keyword,
                                      constructor(view.get_lhs()),
                                      constructor(view.get_mid()),
                                      constructor(view.get_rhs()));
    else if constexpr (runir::kr::dl::NumericalBinaryTag<Tag>)
        return constructor_components(runir::kr::dl::numerical_binary_keyword<Tag>(), constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NumericalConstantTag>)
        return constructor_components(runir::kr::dl::numerical_constant_keyword, fmt::format("{}", view.get_value()));
    else
        static_assert(ygg::dependent_false<Tag>::value, "unhandled DL numerical constructor tag in numerical");
}

template<runir::kr::dl::BaseConceptConstructorTag Tag, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, Tag>>, C> view)
{
    return concept_constructor(view);
}

template<runir::kr::dl::BaseRoleConstructorTag Tag, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Role<runir::kr::UnsFamilyTag, Tag>>, C> view)
{
    return role(view);
}

template<runir::kr::dl::UnsBooleanConstructorTag Tag, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, Tag>>, C> view)
{
    return boolean_constructor(view);
}

template<runir::kr::dl::UnsNumericalConstructorTag Tag, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, Tag>>, C> view)
{
    return numerical(view);
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, Category>>, C> view)
{
    return ygg::visit([](auto arg) { return constructor_body(arg); }, view.get_variant());
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(ygg::View<ygg::Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, Category>>, C> view)
{
    return fmt::format("({})", constructor_body(view));
}

template<typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::uns::dl::Feature>, C> view)
{
    os << "(:" << runir::kr::uns::dl::Feature::keyword << "\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << fmt::format("(:symbol {})", std::string(view.get_symbol().str())) << "\n";
        os << ygg::print_indent << fmt::format("(:description {})", fmt::format("{:?}", std::string(view.get_description().str()))) << "\n";
        os << ygg::print_indent << "(:expression\n";
        {
            ygg::IndentScope expression_scope(os);
            append_component(os, constructor(view.get_feature()));
            os << "\n";
        }
        os << ygg::print_indent << ")\n";
    }
    os << ygg::print_indent << ")";
}

template<typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::uns::dl::Feature>, C> view)
{
    auto os = std::ostringstream {};
    append_feature(os, view);
    return os.str();
}

}  // namespace runir::kr::uns::dl::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::uns::dl::Feature>, C>> : fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::uns::dl::Feature>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::uns::dl::format::feature(view), ctx); }
};
#endif

#endif
