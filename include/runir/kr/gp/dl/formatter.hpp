#ifndef RUNIR_KR_GP_DL_FORMATTER_HPP_
#define RUNIR_KR_GP_DL_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/gp/dl/condition_view.hpp"
#include "runir/kr/gp/dl/effect_view.hpp"
#include "runir/kr/gp/dl/feature_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

namespace runir::kr::gp::dl::format
{

template<typename String>
std::string quoted(const String& value)
{
    return fmt::format("\"{}\"", value.str());
}

inline std::string quoted(std::string_view value) { return fmt::format("\"{}\"", value); }

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C> view);

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C> view);

template<typename View>
std::string constructor_variant(View view)
{
    return view.apply([](auto arg) { return constructor(arg); });
}

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::string concept_constructor(tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return std::string(runir::kr::dl::grammar::ast::ConceptBot::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return std::string(runir::kr::dl::grammar::ast::ConceptTop::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptIntersection::keyword, constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptUnion::keyword, constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNegation::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::ConceptValueRestriction::keyword, constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptExistentialQuantification::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapContainmentTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptRoleValueMapContainment::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapEqualityTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptRoleValueMapEquality::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNominal::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return std::string(runir::kr::dl::grammar::ast::RoleUniversal::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleIntersection::keyword, constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleUnion::keyword, constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleComplement::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleInverse::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleComposition::keyword, constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleTransitiveClosure::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("{} {} {}", runir::kr::dl::grammar::ast::RoleRestriction::keyword, constructor(view.get_lhs()), constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleIdentity::keyword, constructor(view.get_arg()));
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicState::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::BooleanNonempty::keyword, constructor_variant(view.get_arg()));
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::NumericalCount::keyword, constructor_variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::NumericalDistance::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_mid()),
                           constructor(view.get_rhs()));
}

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C> view)
{
    return concept_constructor(view);
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C> view)
{
    return role(view);
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C> view)
{
    return boolean_constructor(view);
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C> view)
{
    return numerical(view);
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C> view)
{
    return view.get_variant().apply([](auto arg) { return constructor_body(arg); });
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C> view)
{
    return fmt::format("({})", constructor_body(view));
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os, tyr::View<tyr::Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> view, std::string_view name)
{
    os << "(" << FeatureTag::keyword << " " << name << " " << quoted(view.get_symbol()) << " " << quoted(view.get_description()) << " "
       << constructor(view.get_feature()) << ")";
}

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> view, std::string_view name)
{
    auto os = std::ostringstream {};
    append_feature(os, view, name);
    return os.str();
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(tyr::View<tyr::Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C>,
                      std::string_view feature_name)
{
    auto os = std::ostringstream {};
    os << "(" << ObservationTag::keyword << " " << feature_name << ")";
    return os.str();
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(tyr::View<tyr::Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C>, std::string_view feature_name)
{
    auto os = std::ostringstream {};
    os << "(" << ObservationTag::keyword << " " << feature_name << ")";
    return os.str();
}

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> view)
{
    return feature(view, fmt::format("f_{}", view.get_index().get_value()));
}

}  // namespace runir::kr::gp::dl::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<typename FeatureTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::gp::dl::format::feature(view), ctx); }
};

template<typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = runir::kr::gp::dl::format::condition(view, fmt::format("f_{}", view.get_feature().get_index().get_value()));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = runir::kr::gp::dl::format::effect(view, fmt::format("f_{}", view.get_feature().get_index().get_value()));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};
#endif

#endif
