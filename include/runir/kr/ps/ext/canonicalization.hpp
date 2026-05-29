#ifndef RUNIR_KR_PS_EXT_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_EXT_CANONICALIZATION_HPP_

#include "runir/kr/ps/ext/argument_data.hpp"
#include "runir/kr/ps/ext/condition_data.hpp"
#include "runir/kr/ps/ext/effect_data.hpp"
#include "runir/kr/ps/ext/feature_data.hpp"
#include "runir/kr/ps/ext/memory_state_data.hpp"
#include "runir/kr/ps/ext/memory_transition_data.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/module_program_data.hpp"
#include "runir/kr/ps/ext/register_data.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_variant_data.hpp"

#include <algorithm>
#include <tyr/common/canonicalization.hpp>
#include <tyr/common/comparators.hpp>
#include <tyr/common/equal_to.hpp>

namespace runir::kr::ps::ext
{

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const tyr::Data<Argument<Category>>&) noexcept
{
    return true;
}

inline bool is_canonical(const tyr::Data<MemoryState>&) noexcept { return true; }

inline bool is_canonical(const tyr::Data<Register>&) noexcept { return true; }

template<typename FeatureTag>
bool is_canonical(const tyr::Data<Feature<FeatureTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag>
bool is_canonical(const tyr::Data<ConcreteFeature<runir::kr::DlTag, FeatureTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag, typename ObservationTag>
bool is_canonical(const tyr::Data<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag, typename ObservationTag>
bool is_canonical(const tyr::Data<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

inline bool is_canonical(const tyr::Data<ConditionVariant>&) noexcept { return true; }

inline bool is_canonical(const tyr::Data<ConcreteConditionVariant<runir::kr::DlTag>>&) noexcept { return true; }

inline bool is_canonical(const tyr::Data<EffectVariant>&) noexcept { return true; }

inline bool is_canonical(const tyr::Data<ConcreteEffectVariant<runir::kr::DlTag>>&) noexcept { return true; }

template<RuleKind Kind>
bool is_canonical(const tyr::Data<Rule<Kind>>& data) noexcept
{
    if constexpr (std::same_as<Kind, LoadTag>)
        return tyr::is_canonical(data.conditions);
    else if constexpr (std::same_as<Kind, SketchTag>)
        return tyr::is_canonical(data.conditions) && tyr::is_canonical(data.effects);
    else if constexpr (std::same_as<Kind, DoTag>)
        return tyr::is_canonical(data.conditions);
    else if constexpr (std::same_as<Kind, CallTag>)
        return tyr::is_canonical(data.conditions);
}

inline bool is_canonical(const tyr::Data<RuleVariant>&) noexcept { return true; }

inline bool is_canonical(const tyr::Data<MemoryTransition>& transition) noexcept { return tyr::is_canonical(transition.rules); }

inline bool is_canonical(const tyr::Data<Module>& data) noexcept
{
    return tyr::is_canonical(data.concept_arguments) && tyr::is_canonical(data.role_arguments) && tyr::is_canonical(data.boolean_arguments)
           && tyr::is_canonical(data.numerical_arguments) && tyr::is_canonical(data.registers) && tyr::is_canonical(data.memory_states);
}

inline bool is_canonical(const tyr::Data<ModuleProgram>&) noexcept { return true; }

template<runir::kr::dl::CategoryTag Category>
void canonicalize(tyr::Data<Argument<Category>>&) noexcept
{
}

inline void canonicalize(tyr::Data<MemoryState>&) noexcept {}

inline void canonicalize(tyr::Data<Register>&) noexcept {}

template<typename FeatureTag>
void canonicalize(tyr::Data<Feature<FeatureTag>>&) noexcept
{
}

template<typename FeatureTag>
void canonicalize(tyr::Data<ConcreteFeature<runir::kr::DlTag, FeatureTag>>&) noexcept
{
}

template<typename FeatureTag, typename ObservationTag>
void canonicalize(tyr::Data<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
}

template<typename FeatureTag, typename ObservationTag>
void canonicalize(tyr::Data<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
}

inline void canonicalize(tyr::Data<ConditionVariant>&) noexcept {}

inline void canonicalize(tyr::Data<ConcreteConditionVariant<runir::kr::DlTag>>&) noexcept {}

inline void canonicalize(tyr::Data<EffectVariant>&) noexcept {}

inline void canonicalize(tyr::Data<ConcreteEffectVariant<runir::kr::DlTag>>&) noexcept {}

template<RuleKind Kind>
void canonicalize(tyr::Data<Rule<Kind>>& data)
{
    if constexpr (std::same_as<Kind, LoadTag>)
    {
        tyr::canonicalize(data.conditions);
    }
    else if constexpr (std::same_as<Kind, SketchTag>)
    {
        tyr::canonicalize(data.conditions);
        tyr::canonicalize(data.effects);
    }
    else if constexpr (std::same_as<Kind, DoTag>)
    {
        tyr::canonicalize(data.conditions);
    }
    else if constexpr (std::same_as<Kind, CallTag>)
    {
        tyr::canonicalize(data.conditions);
    }
}

inline void canonicalize(tyr::Data<RuleVariant>&) noexcept {}

inline void canonicalize(tyr::Data<MemoryTransition>& transition) { tyr::canonicalize(transition.rules); }

inline void canonicalize(tyr::Data<Module>& data)
{
    tyr::canonicalize(data.concept_arguments);
    tyr::canonicalize(data.role_arguments);
    tyr::canonicalize(data.boolean_arguments);
    tyr::canonicalize(data.numerical_arguments);
    tyr::canonicalize(data.registers);
    tyr::canonicalize(data.memory_states);
}

inline void canonicalize(tyr::Data<ModuleProgram>&) noexcept {}

}  // namespace runir::kr::ps::ext

#endif
