#include "detail.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace runir::kr::ps::ext::dl::detail
{

template<typename Features, typename Feature>
std::size_t feature_position(const Features& features, Feature feature)
{
    return static_cast<std::size_t>(
        std::distance(features.begin(),
                      std::find_if(features.begin(), features.end(), [&](auto candidate) { return candidate.get_index() == feature.get_index(); })));
}

template<runir::kr::dl::CategoryTag Category>
using RegisterConstructor = std::conditional_t<std::same_as<Category, runir::kr::dl::ConceptTag>,
                                               runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                               runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>;

template<runir::kr::dl::CategoryTag Category>
using RegisterConstructorView = ygg::View<ygg::Index<RegisterConstructor<Category>>, runir::kr::dl::ExtConstructorRepository>;

template<runir::kr::dl::CategoryTag Category, typename VariantView>
bool variant_references_register(VariantView view, runir::kr::dl::RegisterIdentifier<Category> reg);

template<runir::kr::dl::CategoryTag Category, typename View>
bool references_register(View view, runir::kr::dl::RegisterIdentifier<Category> reg)
{
    if constexpr (requires { view.get_variant(); })
    {
        return variant_references_register(view.get_variant(), reg);
    }
    else
    {
        if constexpr (std::same_as<View, RegisterConstructorView<Category>>)
            return view.get_register().get_identifier() == reg;

        auto result = false;
        if constexpr (requires { view.get_arg(); })
        {
            auto arg = view.get_arg();
            if constexpr (requires { arg.get_variant(); })
                result = result || references_register(arg, reg);
            else
                result = result || variant_references_register(arg, reg);
        }
        if constexpr (requires { view.get_lhs(); })
            result = result || references_register(view.get_lhs(), reg);
        if constexpr (requires { view.get_rhs(); })
            result = result || references_register(view.get_rhs(), reg);
        if constexpr (requires { view.get_mid(); })
            result = result || references_register(view.get_mid(), reg);
        if constexpr (requires { view.get_role(); })
            result = result || references_register(view.get_role(), reg);
        if constexpr (requires { view.get_concept(); })
            result = result || references_register(view.get_concept(), reg);
        return result;
    }
}

template<runir::kr::dl::CategoryTag Category, typename VariantView>
bool variant_references_register(VariantView view, runir::kr::dl::RegisterIdentifier<Category> reg)
{
    return ygg::visit([&](auto child) { return references_register(child, reg); }, view);
}

template<typename FeatureTag, typename C, runir::kr::dl::CategoryTag Category>
bool feature_references_register(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> feature,
                                 runir::kr::dl::RegisterIdentifier<Category> reg)
{
    return ygg::visit([&](auto concrete) { return references_register(concrete.get_expression(), reg); }, feature.get_variant());
}

template<runir::kr::dl::CategoryTag Category>
void record_load_effects(ModuleView module_, runir::kr::ps::detail::RuleProfile& profile, runir::kr::dl::RegisterIdentifier<Category> reg)
{
    const auto booleans = module_.get_features<runir::kr::ps::dl::BooleanFeature>();
    const auto numericals = module_.get_features<runir::kr::ps::dl::NumericalFeature>();
    for (std::size_t position = 0; position < booleans.size(); ++position)
        if (!feature_references_register(booleans[position], reg))
            profile.template effects<runir::kr::ps::dl::Unchanged>().set(position);

    for (std::size_t position = 0; position < numericals.size(); ++position)
        if (!feature_references_register(numericals[position], reg))
            profile.numerical_changes[position] = runir::kr::ps::dl::NumericalChange::UNCHANGED;
}

template<typename FeatureTag, typename ObservationTag>
void record_condition(
    ModuleView module_,
    runir::kr::ps::detail::RuleProfile& profile,
    ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> condition)
{
    namespace psdl = runir::kr::ps::dl;
    const auto position = [&]()
    {
        if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
            return feature_position(module_.get_features<psdl::BooleanFeature>(), condition.get_feature());
        else
            return feature_position(module_.get_features<psdl::NumericalFeature>(), condition.get_feature());
    }();
    profile.template conditions<ObservationTag>().set(position);
}

template<typename FeatureTag, typename ObservationTag>
void record_effect(
    ModuleView module_,
    runir::kr::ps::detail::RuleProfile& profile,
    ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> effect)
{
    namespace psdl = runir::kr::ps::dl;
    if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
    {
        const auto position = feature_position(module_.get_features<psdl::BooleanFeature>(), effect.get_feature());
        profile.template effects<ObservationTag>().set(position);
    }
    else
    {
        const auto position = feature_position(module_.get_features<psdl::NumericalFeature>(), effect.get_feature());
        profile.numerical_changes[position] = runir::kr::ps::detail::RuleProfile::template numerical_change<ObservationTag>();
    }
}

ModuleAnalysis analyze_module(ModuleView module_)
{
    auto memory_states = std::vector<MemoryStateView> {};
    for (auto memory_state : module_.get_memory_states())
        memory_states.push_back(memory_state);

    auto rules = std::vector<RuleVariantView> {};
    for (auto transition : module_.get_memory_transitions())
        for (auto rule : transition)
            rules.push_back(rule);

    const auto booleans = module_.get_features<runir::kr::ps::dl::BooleanFeature>();
    const auto numericals = module_.get_features<runir::kr::ps::dl::NumericalFeature>();
    auto policy = runir::kr::ps::detail::QualitativePolicy(memory_states.size(), booleans.size(), numericals.size());
    auto analysis = ModuleAnalysis { std::move(memory_states), std::move(rules), std::move(policy) };
    const auto memory_position = [&](ygg::Index<MemoryState> memory_state)
    {
        for (std::size_t position = 0; position < analysis.memory_states.size(); ++position)
            if (analysis.memory_states[position].get_index() == memory_state)
                return position;
        throw std::logic_error("structural_termination: rule references a memory state not listed in the module.");
    };

    const auto num_booleans = booleans.size();
    const auto num_numericals = numericals.size();
    for (auto rule : analysis.rules)
    {
        auto profile = runir::kr::ps::detail::RuleProfile(num_booleans, num_numericals);
        ygg::visit(
            [&](auto concrete_rule)
            {
                profile.source_memory_position = memory_position(concrete_rule.get_source().get_index());
                profile.target_memory_position = memory_position(concrete_rule.get_target().get_index());
                for (auto condition : concrete_rule.get_conditions())
                    ygg::visit([&](auto concrete_variant)
                               { ygg::visit([&](auto concrete) { record_condition(module_, profile, concrete); }, concrete_variant.get_variant()); },
                               condition.get_variant());
                if constexpr (requires { concrete_rule.get_effects(); })
                {
                    for (auto effect : concrete_rule.get_effects())
                        ygg::visit([&](auto concrete_variant)
                                   { ygg::visit([&](auto concrete) { record_effect(module_, profile, concrete); }, concrete_variant.get_variant()); },
                                   effect.get_variant());
                }
                if constexpr (requires { concrete_rule.get_register(); })
                    record_load_effects(module_, profile, concrete_rule.get_register().get_identifier());
                // Rules without explicit effect entries leave unmentioned
                // features unconstrained. Call rules have no effect entries.
            },
            rule.get_variant());
        analysis.policy.rule_profiles.push_back(std::move(profile));
    }

    return analysis;
}

}  // namespace runir::kr::ps::ext::dl::detail
