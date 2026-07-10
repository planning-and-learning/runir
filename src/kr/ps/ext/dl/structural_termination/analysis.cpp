#include "detail.hpp"

#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace runir::kr::ps::ext::dl::detail
{

template<typename FeatureTag>
std::size_t feature_position(const std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>>& features,
                             ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature)
{
    return static_cast<std::size_t>(std::distance(features.begin(), std::find(features.begin(), features.end(), feature)));
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
            return view.get_data().identifier == reg;

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

template<typename FeatureTag, runir::kr::dl::CategoryTag Category>
bool feature_references_register(ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>> feature,
                                 const Repository& repository,
                                 runir::kr::dl::RegisterIdentifier<Category> reg)
{
    return ygg::visit([&](auto concrete) { return references_register(concrete.get_expression(), reg); }, ygg::make_view(feature, repository).get_variant());
}

template<runir::kr::dl::CategoryTag Category>
void record_load_effects(const ModuleFeatures& features,
                         ModuleRuleProfile& profile,
                         const Repository& repository,
                         runir::kr::dl::RegisterIdentifier<Category> reg)
{
    for (std::size_t position = 0; position < features.booleans.size(); ++position)
        if (!feature_references_register(features.booleans[position], repository, reg))
            profile.template effects<runir::kr::ps::dl::Unchanged>().set(position);

    for (std::size_t position = 0; position < features.numericals.size(); ++position)
        if (!feature_references_register(features.numericals[position], repository, reg))
            profile.numerical_changes[position] = NumericalChange::UNCHANGED;
}

template<typename FeatureTag, typename ObservationTag>
void record_condition(
    const ModuleFeatures& features,
    ModuleRuleProfile& profile,
    ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> condition)
{
    namespace psdl = runir::kr::ps::dl;
    const auto position = [&]()
    {
        if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
            return feature_position(features.booleans, condition.get_feature().get_index());
        else
            return feature_position(features.numericals, condition.get_feature().get_index());
    }();
    profile.template conditions<ObservationTag>().set(position);
}

template<typename FeatureTag, typename ObservationTag>
void record_effect(
    const ModuleFeatures& features,
    ModuleRuleProfile& profile,
    ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> effect)
{
    namespace psdl = runir::kr::ps::dl;
    if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
    {
        const auto position = feature_position(features.booleans, effect.get_feature().get_index());
        profile.template effects<ObservationTag>().set(position);
    }
    else
    {
        const auto position = feature_position(features.numericals, effect.get_feature().get_index());
        profile.numerical_changes[position] = ModuleRuleProfile::template numerical_change<ObservationTag>();
    }
}

ModuleAnalysis analyze_module(ModuleView module_)
{
    auto analysis = ModuleAnalysis {};

    for (auto feature : module_.get_features<runir::kr::ps::dl::BooleanFeature>())
        analysis.features.booleans.push_back(feature.get_index());
    for (auto feature : module_.get_features<runir::kr::ps::dl::NumericalFeature>())
        analysis.features.numericals.push_back(feature.get_index());

    for (auto memory_state : module_.get_memory_states())
        analysis.memory_states.push_back(memory_state);
    const auto memory_position = [&](ygg::Index<MemoryState> memory_state)
    {
        for (std::size_t position = 0; position < analysis.memory_states.size(); ++position)
            if (analysis.memory_states[position].get_index() == memory_state)
                return position;
        throw std::logic_error("structural_termination: rule references a memory state not listed in the module.");
    };

    for (auto transition : module_.get_memory_transitions())
        for (auto rule : transition)
            analysis.rules.push_back(rule);

    const auto num_booleans = analysis.features.booleans.size();
    const auto num_numericals = analysis.features.numericals.size();
    for (auto rule : analysis.rules)
    {
        auto profile = ModuleRuleProfile(num_booleans, num_numericals);
        ygg::visit(
            [&](auto concrete_rule)
            {
                profile.source_memory_position = memory_position(concrete_rule.get_source().get_index());
                profile.target_memory_position = memory_position(concrete_rule.get_target().get_index());
                for (auto condition : concrete_rule.get_conditions())
                    ygg::visit([&](auto concrete_variant)
                               { ygg::visit([&](auto concrete) { record_condition(analysis.features, profile, concrete); }, concrete_variant.get_variant()); },
                               condition.get_variant());
                if constexpr (requires { concrete_rule.get_effects(); })
                {
                    for (auto effect : concrete_rule.get_effects())
                        ygg::visit([&](auto concrete_variant)
                                   { ygg::visit([&](auto concrete) { record_effect(analysis.features, profile, concrete); }, concrete_variant.get_variant()); },
                                   effect.get_variant());
                }
                if constexpr (requires { concrete_rule.get_register(); })
                    record_load_effects(analysis.features, profile, module_.get_context(), concrete_rule.get_register().get_identifier());
                // Rules without explicit effect entries leave unmentioned
                // features unconstrained. Call rules have no effect entries.
            },
            rule.get_variant());
        analysis.profiles.push_back(std::move(profile));
    }

    return analysis;
}

}  // namespace runir::kr::ps::ext::dl::detail
