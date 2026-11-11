#ifndef RUNIR_KR_PARSER_OBSERVATIONS_HPP_
#define RUNIR_KR_PARSER_OBSERVATIONS_HPP_

#include "runir/kr/parser/diagnostics.hpp"
#include "runir/kr/ps/base/dl/ast/ast.hpp"
#include "runir/kr/ps/dl/condition_data.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_data.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <concepts>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::parser::observations
{

template<runir::kr::FamilyTag Family, typename FeatureTag>
auto require_feature(const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, FeatureTag>>>, urd::stable_refs>& features,
                     const runir::kr::parser::ast::Identifier& name,
                     const runir::kr::parser::DiagnosticContext& diagnostics)
{
    const auto it = features.find(name.text);
    if (it == features.end())
        diagnostics.throw_at(name, runir::kr::UndefinedSymbolError("feature", name.text));
    return it->second;
}

template<typename FeatureTag, typename ObservationTag, runir::kr::FamilyTag Family, typename Repository, typename Builder>
auto make_condition(Repository& repository, Builder& builder, urd::index<runir::kr::ps::Feature<Family, FeatureTag>> feature)
{
    auto concrete_data = builder.template checkout<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>();
    concrete_data->feature = feature;
    return runir::kr::ps::ConditionVariant<Family>(repository.insert(*concrete_data).first.index());
}

template<typename FeatureTag, typename ObservationTag, runir::kr::FamilyTag Family, typename Repository, typename Builder>
auto make_effect(Repository& repository, Builder& builder, urd::index<runir::kr::ps::Feature<Family, FeatureTag>> feature)
{
    auto concrete_data = builder.template checkout<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>();
    concrete_data->feature = feature;
    return runir::kr::ps::EffectVariant<Family>(repository.insert(*concrete_data).first.index());
}

template<runir::kr::FamilyTag Family, typename Repository, typename Builder>
auto parse_condition(
    Repository& repository,
    Builder& builder,
    const runir::kr::ps::base::dl::ast::Condition& condition,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, runir::kr::dl::BooleanTag>>>, urd::stable_refs>& boolean_features,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, runir::kr::dl::NumericalTag>>>, urd::stable_refs>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&]<typename Observation>([[maybe_unused]] const Observation& observation)
        {
            if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Positive>)
            {
                return make_condition<runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>(
                    repository,
                    builder,
                    require_feature(boolean_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Negative>)
            {
                return make_condition<runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>(
                    repository,
                    builder,
                    require_feature(boolean_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::EqualZero>)
            {
                return make_condition<runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>(
                    repository,
                    builder,
                    require_feature(numerical_features, condition.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::GreaterZero>)
            {
                return make_condition<runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>(
                    repository,
                    builder,
                    require_feature(numerical_features, condition.feature, diagnostics));
            }
        },
        condition.observation.get());
}

template<runir::kr::FamilyTag Family, typename Repository, typename Builder>
auto parse_effect(
    Repository& repository,
    Builder& builder,
    const runir::kr::ps::base::dl::ast::Effect& effect,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, runir::kr::dl::BooleanTag>>>, urd::stable_refs>& boolean_features,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, runir::kr::dl::NumericalTag>>>, urd::stable_refs>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics)
{
    return boost::apply_visitor(
        [&]<typename Observation>([[maybe_unused]] const Observation& observation)
        {
            if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Positive>)
            {
                return make_effect<runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>(repository,
                                                                                           builder,
                                                                                           require_feature(boolean_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Negative>)
            {
                return make_effect<runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>(repository,
                                                                                           builder,
                                                                                           require_feature(boolean_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Unchanged>)
            {
                if (boolean_features.contains(effect.feature.text) && numerical_features.contains(effect.feature.text))
                    diagnostics.throw_at(effect.feature, runir::kr::InvalidExpressionError("Ambiguous feature \"" + effect.feature.text + "\"."));
                if (boolean_features.contains(effect.feature.text))
                    return make_effect<runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>(repository,
                                                                                                builder,
                                                                                                require_feature(boolean_features, effect.feature, diagnostics));
                return make_effect<runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>(repository,
                                                                                              builder,
                                                                                              require_feature(numerical_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Increases>)
            {
                return make_effect<runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>(repository,
                                                                                              builder,
                                                                                              require_feature(numerical_features, effect.feature, diagnostics));
            }
            else if constexpr (std::same_as<Observation, runir::kr::ps::base::dl::ast::Decreases>)
            {
                return make_effect<runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>(repository,
                                                                                              builder,
                                                                                              require_feature(numerical_features, effect.feature, diagnostics));
            }
        },
        effect.observation.get());
}

template<runir::kr::FamilyTag Family, typename Repository, typename Builder>
void append_conditions(
    Repository& repository,
    Builder& builder,
    const urd::Store<urd::sequence<runir::kr::ps::base::dl::ast::Condition>>& observations,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, runir::kr::dl::BooleanTag>>>, urd::stable_refs>& boolean_features,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, runir::kr::dl::NumericalTag>>>, urd::stable_refs>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics,
    runir::kr::ps::ConditionList<Family>& result)
{
    result.reserve(result.size() + observations.size());
    for (const auto& observation : observations)
        result.push_back(parse_condition(repository, builder, observation, boolean_features, numerical_features, diagnostics));
}

template<runir::kr::FamilyTag Family, typename Repository, typename Builder>
void append_effects(
    Repository& repository,
    Builder& builder,
    const urd::Store<urd::sequence<runir::kr::ps::base::dl::ast::Effect>>& observations,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, runir::kr::dl::BooleanTag>>>, urd::stable_refs>& boolean_features,
    const urd::Store<urd::map<std::string, urd::index<runir::kr::ps::Feature<Family, runir::kr::dl::NumericalTag>>>, urd::stable_refs>& numerical_features,
    const runir::kr::parser::DiagnosticContext& diagnostics,
    runir::kr::ps::EffectList<Family>& result)
{
    result.reserve(result.size() + observations.size());
    for (const auto& observation : observations)
        result.push_back(parse_effect(repository, builder, observation, boolean_features, numerical_features, diagnostics));
}

}  // namespace runir::kr::parser::observations

#endif
