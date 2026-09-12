#include "detail.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace runir::kr::ps::ext::dl::detail
{

using runir::kr::ps::detail::record_condition;
using runir::kr::ps::detail::record_effect;

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
        {
            profile.boolean_unconstrained_effects &= ~(std::uint64_t { 1 } << position);
            profile.boolean_unchanged_effects |= std::uint64_t { 1 } << position;
        }

    for (std::size_t position = 0; position < numericals.size(); ++position)
        if (!feature_references_register(numericals[position], reg))
        {
            profile.numerical_unconstrained_effects &= ~(std::uint64_t { 1 } << position);
            profile.numerical_unchanged_effects |= std::uint64_t { 1 } << position;
        }
}

Analysis analyze_module(ModuleView module_)
{
    const auto memory_states = module_.get_memory_states();

    auto rules = std::vector<RuleVariantView> {};
    for (auto transition : module_.get_memory_transitions())
        for (auto rule : transition)
            rules.push_back(rule);

    const auto booleans = module_.get_features<runir::kr::ps::dl::BooleanFeature>();
    const auto numericals = module_.get_features<runir::kr::ps::dl::NumericalFeature>();
    auto policy = runir::kr::ps::detail::QualitativePolicy(memory_states.size(), booleans.size(), numericals.size());
    auto analysis = Analysis { std::move(rules), std::move(policy) };
    const auto memory_position = [&](ygg::Index<MemoryState> memory_state)
    {
        for (std::size_t position = 0; position < memory_states.size(); ++position)
            if (memory_states[position].get_index() == memory_state)
                return position;
        throw std::logic_error("structural_termination: rule references a memory state not listed in the module.");
    };

    for (auto rule : analysis.rules)
    {
        auto profile = runir::kr::ps::detail::RuleProfile(booleans.size(), numericals.size());
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

    runir::kr::ps::detail::validate_policy(analysis.policy);
    return analysis;
}

}  // namespace runir::kr::ps::ext::dl::detail
