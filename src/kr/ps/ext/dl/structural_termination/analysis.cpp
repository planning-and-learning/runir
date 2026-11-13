#include "detail.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/elements/view.hpp>
#include <utility>
#include <variant>

namespace runir::kr::ps::ext::dl::detail
{
namespace
{

template<runir::kr::dl::CategoryTag Category>
using RegisterConstructor = std::conditional_t<std::same_as<Category, runir::kr::dl::ConceptTag>,
                                               runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::RegisterTag>,
                                               runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::RegisterTag>>;

template<runir::kr::dl::CategoryTag Category>
using RegisterConstructorView = urd::index_view<RegisterConstructor<Category>, runir::kr::dl::ConstructorRepository>;

/// A view of a record that declares its fields (urd::Fields).
template<typename V>
concept RecordView = requires(const V& view) { view.data(); } && urd::HasFields<urd::record_of<V>>;

/// A range of children: any range but text.
template<typename T>
concept ChildRange = std::ranges::range<T> && !std::same_as<std::remove_cvref_t<std::ranges::range_value_t<T>>, char>;

template<typename T>
inline constexpr bool is_variant = false;
template<typename... Ts>
inline constexpr bool is_variant<std::variant<Ts...>> = true;

/// Whether `value` refers to the register `reg`: a register constructor of it; a record through any of its fields; a
/// variant through its alternative; a range through any element. Anything else (a number, a name, a planning symbol)
/// holds no record.
template<runir::kr::dl::CategoryTag Category, typename Value>
bool references_register(const Value& value, runir::kr::dl::RegisterIdentifier<Category> reg)
{
    if constexpr (std::same_as<Value, RegisterConstructorView<Category>>)
        return value.get_register().get_identifier() == reg;
    else if constexpr (is_variant<Value>)
        return std::visit([&](const auto& child) { return references_register<Category>(child, reg); }, value);
    else if constexpr (RecordView<Value>)
    {
        auto result = false;
        urd::for_each_field<urd::record_of<Value>>([&]<typename F>()
                                                   { result = result || references_register<Category>(value.template member<F::member>(), reg); });
        return result;
    }
    else if constexpr (ChildRange<Value>)
    {
        for (const auto& child : value)
            if (references_register<Category>(child, reg))
                return true;
        return false;
    }
    else
        return false;
}

template<typename FeatureTag, typename C, runir::kr::dl::CategoryTag Category>
bool feature_references_register(urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>, C> feature,
                                 runir::kr::dl::RegisterIdentifier<Category> reg)
{
    return std::visit([&](auto concrete) { return references_register(concrete.get_expression(), reg); }, feature.get_variant());
}

template<runir::kr::dl::CategoryTag Category>
void record_binding_effects(ModuleView module_, runir::kr::ps::detail::RuleProfile& profile, runir::kr::dl::RegisterIdentifier<Category> reg)
{
    const auto booleans = module_.get_features<runir::kr::dl::BooleanTag>();
    const auto numericals = module_.get_features<runir::kr::dl::NumericalTag>();
    for (std::size_t position = 0; position < booleans.size(); ++position)
        if ((profile.boolean_unconstrained_effects & (std::uint64_t { 1 } << position))
            && !feature_references_register(booleans[static_cast<std::ptrdiff_t>(position)], reg))
        {
            profile.boolean_unconstrained_effects &= ~(std::uint64_t { 1 } << position);
            profile.boolean_unchanged_effects |= std::uint64_t { 1 } << position;
        }

    for (std::size_t position = 0; position < numericals.size(); ++position)
        if ((profile.numerical_unconstrained_effects & (std::uint64_t { 1 } << position))
            && !feature_references_register(numericals[static_cast<std::ptrdiff_t>(position)], reg))
        {
            profile.numerical_unconstrained_effects &= ~(std::uint64_t { 1 } << position);
            profile.numerical_unchanged_effects |= std::uint64_t { 1 } << position;
        }
}

}  // namespace

Analysis analyze_module(ModuleView module_)
{
    const auto memory_states = module_.get_memory_states();

    const auto booleans = module_.get_features<runir::kr::dl::BooleanTag>();
    const auto numericals = module_.get_features<runir::kr::dl::NumericalTag>();
    auto policy = runir::kr::ps::detail::QualitativePolicy(memory_states.size(), booleans.size(), numericals.size());
    auto analysis = Analysis { {}, std::move(policy) };
    const auto memory_position = [&](MemoryStateView memory_state)
    {
        for (std::size_t position = 0; position < memory_states.size(); ++position)
            if (memory_states[static_cast<std::ptrdiff_t>(position)] == memory_state)
                return position;
        throw std::logic_error("structural_termination: rule references a memory state not listed in the module.");
    };

    for (auto transition : module_.get_memory_transitions())
        for (auto rule : transition)
            std::visit(
                [&](auto concrete_rule)
                {
                    using Kind = rule_kind_t<decltype(concrete_rule)>;
                    // A Backtrack guard leads to no target: no qualitative transition.
                    if constexpr (TransitionRuleKind<Kind>)
                    {
                        auto profile = runir::kr::ps::detail::RuleProfile(booleans.size(), numericals.size());
                        profile.source_memory_position = memory_position(concrete_rule.get_source());
                        profile.target_memory_position = memory_position(concrete_rule.get_target());
                        for (auto condition : concrete_rule.get_conditions())
                            std::visit([&](auto concrete) { ps::detail::record_condition(module_, profile, concrete); }, condition);
                        if constexpr (EffectRuleKind<Kind>)
                        {
                            for (auto effect : concrete_rule.get_effects())
                                std::visit([&](auto concrete) { ps::detail::record_effect(module_, profile, concrete); }, effect);
                        }
                        if constexpr (BindingRuleKind<Kind>)
                            record_binding_effects(module_, profile, concrete_rule.get_register().get_identifier());
                        // Other rules leave unmentioned features unconstrained.
                        // Call rules have no effect entries.
                        analysis.rules.push_back(rule);
                        analysis.policy.rule_profiles.push_back(profile);
                    }
                },
                rule.get_variant());

    runir::kr::ps::detail::validate_policy(analysis.policy);
    return analysis;
}

}  // namespace runir::kr::ps::ext::dl::detail
