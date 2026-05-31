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
#include <yggdrasil/semantics/canonicalization.hpp>
#include <yggdrasil/semantics/comparators.hpp>
#include <yggdrasil/semantics/equal_to.hpp>

namespace runir::kr::ps::ext
{

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const ygg::Data<Argument<Category>>&) noexcept
{
    return true;
}

inline bool is_canonical(const ygg::Data<MemoryState>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<Register>&) noexcept { return true; }

template<typename FeatureTag>
bool is_canonical(const ygg::Data<Feature<FeatureTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag>
bool is_canonical(const ygg::Data<ConcreteFeature<runir::kr::DlTag, FeatureTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag, typename ObservationTag>
bool is_canonical(const ygg::Data<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag, typename ObservationTag>
bool is_canonical(const ygg::Data<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

inline bool is_canonical(const ygg::Data<ConditionVariant>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<ConcreteConditionVariant<runir::kr::DlTag>>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<EffectVariant>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<ConcreteEffectVariant<runir::kr::DlTag>>&) noexcept { return true; }

template<RuleKind Kind>
bool is_canonical(const ygg::Data<Rule<Kind>>& data) noexcept
{
    if constexpr (std::same_as<Kind, LoadTag>)
        return ygg::is_canonical(data.conditions);
    else if constexpr (std::same_as<Kind, SketchTag>)
        return ygg::is_canonical(data.conditions) && ygg::is_canonical(data.effects);
    else if constexpr (std::same_as<Kind, DoTag>)
        return ygg::is_canonical(data.conditions);
    else if constexpr (std::same_as<Kind, CallTag>)
        return ygg::is_canonical(data.conditions);
}

inline bool is_canonical(const ygg::Data<RuleVariant>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<MemoryTransition>& transition) noexcept { return ygg::is_canonical(transition.rules); }

inline bool is_canonical(const ygg::Data<Module>& data) noexcept
{
    return ygg::is_canonical(data.concept_arguments) && ygg::is_canonical(data.role_arguments) && ygg::is_canonical(data.boolean_arguments)
           && ygg::is_canonical(data.numerical_arguments) && ygg::is_canonical(data.registers) && ygg::is_canonical(data.memory_states);
}

inline bool is_canonical(const ygg::Data<ModuleProgram>&) noexcept { return true; }

template<runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<Argument<Category>>&) noexcept
{
}

inline void canonicalize(ygg::Data<MemoryState>&) noexcept {}

inline void canonicalize(ygg::Data<Register>&) noexcept {}

template<typename FeatureTag>
void canonicalize(ygg::Data<Feature<FeatureTag>>&) noexcept
{
}

template<typename FeatureTag>
void canonicalize(ygg::Data<ConcreteFeature<runir::kr::DlTag, FeatureTag>>&) noexcept
{
}

template<typename FeatureTag, typename ObservationTag>
void canonicalize(ygg::Data<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
}

template<typename FeatureTag, typename ObservationTag>
void canonicalize(ygg::Data<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
}

inline void canonicalize(ygg::Data<ConditionVariant>&) noexcept {}

inline void canonicalize(ygg::Data<ConcreteConditionVariant<runir::kr::DlTag>>&) noexcept {}

inline void canonicalize(ygg::Data<EffectVariant>&) noexcept {}

inline void canonicalize(ygg::Data<ConcreteEffectVariant<runir::kr::DlTag>>&) noexcept {}

template<RuleKind Kind>
void canonicalize(ygg::Data<Rule<Kind>>& data)
{
    if constexpr (std::same_as<Kind, LoadTag>)
    {
        ygg::canonicalize(data.conditions);
    }
    else if constexpr (std::same_as<Kind, SketchTag>)
    {
        ygg::canonicalize(data.conditions);
        ygg::canonicalize(data.effects);
    }
    else if constexpr (std::same_as<Kind, DoTag>)
    {
        ygg::canonicalize(data.conditions);
    }
    else if constexpr (std::same_as<Kind, CallTag>)
    {
        ygg::canonicalize(data.conditions);
    }
}

inline void canonicalize(ygg::Data<RuleVariant>&) noexcept {}

inline void canonicalize(ygg::Data<MemoryTransition>& transition) { ygg::canonicalize(transition.rules); }

inline void canonicalize(ygg::Data<Module>& data)
{
    ygg::canonicalize(data.concept_arguments);
    ygg::canonicalize(data.role_arguments);
    ygg::canonicalize(data.boolean_arguments);
    ygg::canonicalize(data.numerical_arguments);
    ygg::canonicalize(data.registers);
    ygg::canonicalize(data.memory_states);
}

inline void canonicalize(ygg::Data<ModuleProgram>&) noexcept {}

}  // namespace runir::kr::ps::ext

#endif
