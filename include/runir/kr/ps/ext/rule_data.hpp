#ifndef RUNIR_KR_PS_EXT_RULE_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/order_term_data.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <cista/containers/vector.h>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<Category>>>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<Category>>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target;
    runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions;
    urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>> feature;
    urd::index<runir::kr::dl::Register<Category>> reg;
    runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects;

    data() = default;
    // Registers live in the description-logic repository, hence the separate context parameter.
    data(urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target_,
         runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions_,
         urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>> feature_,
         urd::index<runir::kr::dl::Register<Category>> reg_,
         runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        feature(std::move(feature_)),
        reg(std::move(reg_)),
        effects(std::move(effects_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::source, &data::target, &data::conditions, &data::feature, &data::reg, &data::effects> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::source, &data::target, &data::conditions, &data::feature, &data::reg, &data::effects> {}; }
};

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ChooseTag<Category>>>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ChooseTag<Category>>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target;
    runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions;
    urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>> feature;
    urd::index<runir::kr::dl::Register<Category>> reg;
    runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects;

    urd::flat_vector<urd::index<runir::kr::ps::ext::OrderTerm>> order;

    data() = default;
    // Registers live in the description-logic repository, hence the separate context parameter.
    data(urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target_,
         runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions_,
         urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>> feature_,
         urd::index<runir::kr::dl::Register<Category>> reg_,
         runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects_,
         urd::flat_vector<urd::index<runir::kr::ps::ext::OrderTerm>> order_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        feature(std::move(feature_)),
        reg(std::move(reg_)),
        effects(std::move(effects_)),
        order(std::move(order_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::source, &data::target, &data::conditions, &data::feature, &data::reg, &data::effects, &data::order> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::source, &data::target, &data::conditions, &data::feature, &data::reg, &data::effects, &data::order> {}; }
};

template<>
struct data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target;
    runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions;
    runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects;

    data() = default;
    data(urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target_,
         runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions_,
         runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        effects(std::move(effects_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::source, &data::target, &data::conditions, &data::effects> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::source, &data::target, &data::conditions, &data::effects> {}; }
};

template<>
struct data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target;
    runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions;
    runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects;
    urd::flat_string action_name;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>> arguments;

    data() = default;
    data(urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target_,
         runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions_,
         runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects_,
         urd::flat_string action_name_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>> arguments_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        effects(std::move(effects_)),
        action_name(std::move(action_name_)),
        arguments(std::move(arguments_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::source, &data::target, &data::conditions, &data::effects, &data::action_name, &data::arguments> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::source, &data::target, &data::conditions, &data::effects, &data::action_name, &data::arguments> {}; }
};

template<>
struct data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ActionTag>>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ActionTag>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target;
    runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions;
    runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects;
    urd::flat_string action_name;
    urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>> query_feature;

    data() = default;
    data(urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target_,
         runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions_,
         runir::kr::ps::EffectList<runir::kr::ExtFamilyTag> effects_,
         urd::flat_string action_name_,
         urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>> query_feature_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        effects(std::move(effects_)),
        action_name(std::move(action_name_)),
        query_feature(query_feature_)
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::source, &data::target, &data::conditions, &data::effects, &data::action_name, &data::query_feature> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::source, &data::target, &data::conditions, &data::effects, &data::action_name, &data::query_feature> {}; }
};

template<>
struct data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target;
    runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions;
    urd::index<runir::kr::ps::ModuleSymbol<runir::kr::ExtFamilyTag>> callee;
    urd::flat_vector<runir::kr::ps::ext::CallArgument> arguments;

    data() = default;
    data(urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> target_,
         runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions_,
         urd::index<runir::kr::ps::ModuleSymbol<runir::kr::ExtFamilyTag>> callee_,
         urd::flat_vector<runir::kr::ps::ext::CallArgument> arguments_) :
        index(),
        source(source_),
        target(target_),
        conditions(std::move(conditions_)),
        callee(callee_),
        arguments(std::move(arguments_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::source, &data::target, &data::conditions, &data::callee, &data::arguments> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::source, &data::target, &data::conditions, &data::callee, &data::arguments> {}; }
};

template<>
struct data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::BacktrackTag>>
{
    urd::index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::BacktrackTag>> index;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source;
    runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions;

    data() = default;
    data(urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> source_, runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag> conditions_) :
        index(),
        source(source_),
        conditions(std::move(conditions_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::source, &data::conditions> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::source, &data::conditions> {}; }
};

}  // namespace urd

#endif
