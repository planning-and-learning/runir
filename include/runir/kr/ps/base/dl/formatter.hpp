#ifndef RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace runir::kr::ps::base::dl::format
{

template<typename String>
std::string quoted(const String& value)
{
    return fmt::format("\"{}\"", value.str());
}

inline std::string quoted(std::string_view value) { return fmt::format("\"{}\"", value); }

inline std::string boolean(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

template<typename Objects>
std::vector<std::string> quoted_object_names(Objects objects)
{
    auto result = std::vector<std::string> {};
    for (auto object : objects)
        result.push_back(quoted(object.get_name()));
    return result;
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(tyr::View<tyr::Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, Category>>, C> view);

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, Category>>, C> view);

template<typename View>
std::string constructor_variant(View view)
{
    return view.apply([](auto arg) { return constructor(arg); });
}

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::string concept_constructor(tyr::View<tyr::Index<runir::kr::dl::Concept<runir::kr::dl::BaseFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return std::string(runir::kr::dl::grammar::ast::ConceptBot<runir::kr::dl::BaseFamilyTag>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return std::string(runir::kr::dl::grammar::ast::ConceptTop<runir::kr::dl::BaseFamilyTag>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::dl::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::dl::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::dl::BaseFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::dl::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::dl::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::dl::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::dl::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()),
                           constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::dl::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()),
                           constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::dl::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()),
                           constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_role()),
                           fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::dl::BaseFamilyTag>::keyword,
                           fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::dl::BaseFamilyTag>::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<runir::kr::dl::Role<runir::kr::dl::BaseFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return std::string(runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::dl::BaseFamilyTag>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::dl::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::dl::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleUnion<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleComplement<runir::kr::dl::BaseFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleInverse<runir::kr::dl::BaseFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleComposition<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::dl::BaseFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::dl::BaseFamilyTag>::keyword, constructor(view.get_arg()));
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<runir::kr::dl::Boolean<runir::kr::dl::BaseFamilyTag, Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::dl::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::dl::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::dl::BaseFamilyTag>::keyword, constructor_variant(view.get_arg()));
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<runir::kr::dl::Numerical<runir::kr::dl::BaseFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::NumericalCount<runir::kr::dl::BaseFamilyTag>::keyword, constructor_variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::dl::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_mid()),
                           constructor(view.get_rhs()));
}

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Concept<runir::kr::dl::BaseFamilyTag, Tag>>, C> view)
{
    return concept_constructor(view);
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Role<runir::kr::dl::BaseFamilyTag, Tag>>, C> view)
{
    return role(view);
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Boolean<runir::kr::dl::BaseFamilyTag, Tag>>, C> view)
{
    return boolean_constructor(view);
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Numerical<runir::kr::dl::BaseFamilyTag, Tag>>, C> view)
{
    return numerical(view);
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(tyr::View<tyr::Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, Category>>, C> view)
{
    return view.get_variant().apply([](auto arg) { return constructor_body(arg); });
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(tyr::View<tyr::Index<runir::kr::dl::Constructor<runir::kr::dl::BaseFamilyTag, Category>>, C> view)
{
    return fmt::format("({})", constructor_body(view));
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os,
                    tyr::View<tyr::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view,
                    std::string_view name)
{
    os << "(" << FeatureTag::keyword << " " << name << " " << quoted(view.get_symbol()) << " " << quoted(view.get_description()) << " "
       << constructor(view.get_feature()) << ")";
}

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view,
                    std::string_view name)
{
    auto os = std::ostringstream {};
    append_feature(os, view, name);
    return os.str();
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(tyr::View<tyr::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>,
                      std::string_view feature_name)
{
    auto os = std::ostringstream {};
    os << "(" << ObservationTag::keyword << " " << feature_name << ")";
    return os.str();
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(tyr::View<tyr::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>,
                   std::string_view feature_name)
{
    auto os = std::ostringstream {};
    os << "(" << ObservationTag::keyword << " " << feature_name << ")";
    return os.str();
}

template<typename FeatureTag, typename C>
std::string feature(tyr::View<tyr::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    return feature(view, fmt::format("f_{}", view.get_index().get_value()));
}

}  // namespace runir::kr::ps::base::dl::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<typename FeatureTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::dl::format::feature(view), ctx); }
};

template<typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = runir::kr::ps::base::dl::format::condition(view, fmt::format("f_{}", view.get_feature().get_index().get_value()));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = runir::kr::ps::base::dl::format::effect(view, fmt::format("f_{}", view.get_feature().get_index().get_value()));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};
#endif

#endif
