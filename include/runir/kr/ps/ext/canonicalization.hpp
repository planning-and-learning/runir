#ifndef RUNIR_KR_PS_EXT_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_EXT_CANONICALIZATION_HPP_

#include "runir/kr/ps/ext/argument_data.hpp"
#include "runir/kr/ps/ext/condition_data.hpp"
#include "runir/kr/ps/ext/dl/condition_data.hpp"
#include "runir/kr/ps/ext/dl/effect_data.hpp"
#include "runir/kr/ps/ext/effect_data.hpp"
#include "runir/kr/ps/ext/feature_data.hpp"
#include "runir/kr/ps/ext/memory_state_data.hpp"
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

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const ygg::Data<Register<Category>>&) noexcept
{
    return true;
}

template<typename FeatureTag>
bool is_canonical(const ygg::Data<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag>
bool is_canonical(const ygg::Data<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag, typename ObservationTag>
bool is_canonical(const ygg::Data<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

template<typename FeatureTag, typename ObservationTag>
bool is_canonical(const ygg::Data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
    return true;
}

inline bool is_canonical(const ygg::Data<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>&) noexcept { return true; }

template<RuleKind Kind>
bool is_canonical(const ygg::Data<Rule<Kind>>& data) noexcept
{
    if constexpr (std::same_as<Kind, LoadTag<runir::kr::dl::ConceptTag>> || std::same_as<Kind, LoadTag<runir::kr::dl::RoleTag>>)
        return ygg::is_canonical(data.conditions);
    else if constexpr (std::same_as<Kind, SketchTag>)
        return ygg::is_canonical(data.conditions) && ygg::is_canonical(data.effects);
    else if constexpr (std::same_as<Kind, DoTag>)
        return ygg::is_canonical(data.conditions) && ygg::is_canonical(data.effects);
    else if constexpr (std::same_as<Kind, CallTag>)
        return ygg::is_canonical(data.conditions);
}

inline bool is_canonical(const ygg::Data<RuleVariant>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<Module>& data) noexcept
{
    return ygg::is_canonical(data.concept_arguments) && ygg::is_canonical(data.role_arguments) && ygg::is_canonical(data.boolean_arguments)
           && ygg::is_canonical(data.numerical_arguments) && ygg::is_canonical(data.concept_registers) && ygg::is_canonical(data.role_registers)
           && ygg::is_canonical(data.concept_features) && ygg::is_canonical(data.role_features) && ygg::is_canonical(data.boolean_features)
           && ygg::is_canonical(data.numerical_features) && ygg::is_canonical(data.memory_states);
}

inline bool is_canonical(const ygg::Data<ModuleProgram>&) noexcept { return true; }

template<runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<Argument<Category>>&) noexcept
{
}

inline void canonicalize(ygg::Data<MemoryState>&) noexcept {}

template<runir::kr::dl::CategoryTag Category>
void canonicalize(ygg::Data<Register<Category>>&) noexcept
{
}

template<typename FeatureTag>
void canonicalize(ygg::Data<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>&) noexcept
{
}

template<typename FeatureTag>
void canonicalize(ygg::Data<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>&) noexcept
{
}

template<typename FeatureTag, typename ObservationTag>
void canonicalize(ygg::Data<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
}

template<typename FeatureTag, typename ObservationTag>
void canonicalize(ygg::Data<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>&) noexcept
{
}

inline void canonicalize(ygg::Data<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>&) noexcept {}

inline void canonicalize(ygg::Data<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>&) noexcept {}

inline void canonicalize(ygg::Data<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>&) noexcept {}

inline void canonicalize(ygg::Data<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>&) noexcept {}

template<RuleKind Kind>
void canonicalize(ygg::Data<Rule<Kind>>& data)
{
    if constexpr (std::same_as<Kind, LoadTag<runir::kr::dl::ConceptTag>> || std::same_as<Kind, LoadTag<runir::kr::dl::RoleTag>>)
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
        ygg::canonicalize(data.effects);
    }
    else if constexpr (std::same_as<Kind, CallTag>)
    {
        ygg::canonicalize(data.conditions);
    }
}

inline void canonicalize(ygg::Data<RuleVariant>&) noexcept {}

inline void canonicalize(ygg::Data<Module>& data)
{
    ygg::canonicalize(data.concept_arguments);
    ygg::canonicalize(data.role_arguments);
    ygg::canonicalize(data.boolean_arguments);
    ygg::canonicalize(data.numerical_arguments);
    ygg::canonicalize(data.concept_registers);
    ygg::canonicalize(data.role_registers);
    ygg::canonicalize(data.concept_features);
    ygg::canonicalize(data.role_features);
    ygg::canonicalize(data.boolean_features);
    ygg::canonicalize(data.numerical_features);
    ygg::canonicalize(data.memory_states);
}

inline void canonicalize(ygg::Data<ModuleProgram>&) noexcept {}

}  // namespace runir::kr::ps::ext

#endif
