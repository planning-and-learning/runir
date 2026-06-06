#ifndef RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_
#define RUNIR_KR_PS_BASE_DL_FORMATTER_HPP_

#include "runir/config.hpp"
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
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps::base::dl::format
{

template<typename String>
std::string quoted(const String& value)
{
    return fmt::format("\"{}\"", value.str());
}

inline std::string quoted(std::string_view value) { return fmt::format("\"{}\"", value); }

template<typename String>
std::string symbol_section(const String& value)
{
    const auto text = std::string(value.str());
    return text.empty() ? std::string("(:symbol)") : fmt::format("(:symbol {})", text);
}

inline std::string symbol_section(std::string_view value)
{
    return value.empty() ? std::string("(:symbol)") : fmt::format("(:symbol {})", value);
}

inline std::string symbol_section(const std::string& value) { return symbol_section(std::string_view(value)); }

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
std::string constructor(ygg::View<ygg::Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>>, C> view);

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>>, C> view);

template<typename View>
std::string constructor_variant(View view)
{
    return ygg::visit([](auto arg) { return constructor(arg); }, view);
}

template<runir::kr::dl::BaseConceptConstructorTag Tag, typename C>
std::string concept_constructor(ygg::View<ygg::Index<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return std::string(runir::kr::dl::grammar::ast::ConceptBot<runir::kr::BaseFamilyTag>::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return std::string(runir::kr::dl::grammar::ast::ConceptTop<runir::kr::BaseFamilyTag>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicState<runir::kr::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicGoal<runir::kr::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptIntersection<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptUnion<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNegation<runir::kr::BaseFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptValueRestriction<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptExistentialQuantification<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtLeastNumberRestriction<runir::kr::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtMostNumberRestriction<runir::kr::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptExactNumberRestriction<runir::kr::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedAtLeastNumberRestriction<runir::kr::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()),
                           constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedAtMostNumberRestriction<runir::kr::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()),
                           constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::ConceptQualifiedExactNumberRestriction<runir::kr::BaseFamilyTag>::keyword,
                           view.get_n(),
                           constructor(view.get_role()),
                           constructor(view.get_concept()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptRoleValueMap<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::AgreementTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAgreement<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleFillersTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptRoleFillers<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_role()),
                           fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::OneOfTag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::ConceptOneOf<runir::kr::BaseFamilyTag>::keyword,
                           fmt::join(quoted_object_names(view.get_objects()), " "));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::ConceptNominal<runir::kr::BaseFamilyTag>::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::BaseRoleConstructorTag Tag, typename C>
std::string role(ygg::View<ygg::Index<runir::kr::dl::Role<runir::kr::BaseFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return std::string(runir::kr::dl::grammar::ast::RoleUniversal<runir::kr::BaseFamilyTag>::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleAtomicState<runir::kr::BaseFamilyTag>::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicGoal<runir::kr::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleIntersection<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleUnion<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleComplement<runir::kr::BaseFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleInverse<runir::kr::BaseFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleComposition<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleTransitiveClosure<runir::kr::BaseFamilyTag>::keyword, constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("{} {}",
                           runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::RoleRestriction<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_rhs()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::RoleIdentity<runir::kr::BaseFamilyTag>::keyword, constructor(view.get_arg()));
}

template<runir::kr::dl::BaseBooleanConstructorTag Tag, typename C>
std::string boolean_constructor(ygg::View<ygg::Index<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicState<runir::kr::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicGoal<runir::kr::BaseFamilyTag>::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::BooleanNonempty<runir::kr::BaseFamilyTag>::keyword, constructor_variant(view.get_arg()));
}

template<runir::kr::dl::BaseNumericalConstructorTag Tag, typename C>
std::string numerical(ygg::View<ygg::Index<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("{} {}", runir::kr::dl::grammar::ast::NumericalCount<runir::kr::BaseFamilyTag>::keyword, constructor_variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("{} {} {} {}",
                           runir::kr::dl::grammar::ast::NumericalDistance<runir::kr::BaseFamilyTag>::keyword,
                           constructor(view.get_lhs()),
                           constructor(view.get_mid()),
                           constructor(view.get_rhs()));
}

template<runir::kr::dl::BaseConceptConstructorTag Tag, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, Tag>>, C> view)
{
    return concept_constructor(view);
}

template<runir::kr::dl::BaseRoleConstructorTag Tag, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Role<runir::kr::BaseFamilyTag, Tag>>, C> view)
{
    return role(view);
}

template<runir::kr::dl::BaseBooleanConstructorTag Tag, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, Tag>>, C> view)
{
    return boolean_constructor(view);
}

template<runir::kr::dl::BaseNumericalConstructorTag Tag, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, Tag>>, C> view)
{
    return numerical(view);
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(ygg::View<ygg::Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>>, C> view)
{
    return ygg::visit([](auto arg) { return constructor_body(arg); }, view.get_variant());
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor(ygg::View<ygg::Index<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, Category>>, C> view)
{
    return fmt::format("({})", constructor_body(view));
}

template<typename FeatureTag, typename C>
void append_feature(std::ostream& os,
                    ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    os << "(:" << FeatureTag::keyword << "\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << symbol_section(view.get_symbol()) << "\n";
        os << ygg::print_indent << fmt::format("(:description {})", quoted(view.get_description())) << "\n";
        os << ygg::print_indent << fmt::format("(:expression {})", constructor(view.get_feature())) << "\n";
    }
    os << ygg::print_indent << ")";
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view,
                    std::string_view)
{
    auto os = std::ostringstream {};
    append_feature(os, view);
    return os.str();
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>,
                      std::string_view feature_name)
{
    auto os = std::ostringstream {};
    os << "(:" << ObservationTag::keyword << " " << feature_name << ")";
    return os.str();
}

template<typename FeatureTag, typename ObservationTag, typename C>
std::string effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>,
                   std::string_view feature_name)
{
    auto os = std::ostringstream {};
    os << "(:" << ObservationTag::keyword << " " << feature_name << ")";
    return os.str();
}

template<typename FeatureTag, typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    return feature(view, fmt::format("f_{}", view.get_index().get_value()));
}

}  // namespace runir::kr::ps::base::dl::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<typename FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::base::dl::format::feature(view), ctx); }
};

template<typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = runir::kr::ps::base::dl::format::condition(view, fmt::format("f_{}", view.get_feature().get_index().get_value()));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<typename FeatureTag, typename ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = runir::kr::ps::base::dl::format::effect(view, fmt::format("f_{}", view.get_feature().get_index().get_value()));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};
#endif

#endif
