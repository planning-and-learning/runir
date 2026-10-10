#ifndef RUNIR_KR_PS_EXT_RULE_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/register_index.hpp"
#include "runir/kr/ps/condition_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/effect_index.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/module_symbol_index.hpp"
#include "runir/kr/ps/ext/order_term_data.hpp"
#include "runir/kr/ps/ext/rule_index.hpp"
#include "runir/kr/ps/feature_index.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <cista/containers/vector.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::ps::ext
{

using CallArgument = ::cista::offset::variant<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>,
                                              ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>,
                                              ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>,
                                              ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>>;

}  // namespace runir::kr::ps::ext

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<Category>>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<Category>>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>> feature;
    Index<runir::kr::dl::Register<Category>> reg;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects;

    Data() = default;
    // Registers live in the description-logic repository, hence the separate context parameter.
    Data(Index<runir::kr::ps::ext::MemoryState> source_,
         Index<runir::kr::ps::ext::MemoryState> target_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions_,
         Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>> feature_,
         Index<runir::kr::dl::Register<Category>> reg_,
         IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects_) :
        index(),
        source(std::move(source_)),
        target(std::move(target_)),
        conditions(std::move(conditions_)),
        feature(std::move(feature_)),
        reg(std::move(reg_)),
        effects(std::move(effects_))
    {
    }
    template<typename C, typename D>
    Data(::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> source_,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> target_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>>& conditions_,
         ::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>>, C> feature_,
         ::ygg::View<Index<runir::kr::dl::Register<Category>>, D> reg_,
         const std::vector<::ygg::View<Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C>>& effects_) :
        index(),
        source(),
        target(),
        conditions(),
        feature(),
        reg(),
        effects()
    {
        set(source_, source);
        set(target_, target);
        set(conditions_, conditions);
        set(feature_, feature);
        set(reg_, reg);
        set(effects_, effects);
    }

    auto cista_members() noexcept { return std::tie(index, source, target, conditions, feature, reg, effects); }
    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, feature, reg, effects); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, feature, reg, effects); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ChooseTag<Category>>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ChooseTag<Category>>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>> feature;
    Index<runir::kr::dl::Register<Category>> reg;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects;

    IndexList<runir::kr::ps::ext::OrderTerm> order;

    Data() = default;
    // Registers live in the description-logic repository, hence the separate context parameter.
    Data(Index<runir::kr::ps::ext::MemoryState> source_,
         Index<runir::kr::ps::ext::MemoryState> target_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions_,
         Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>> feature_,
         Index<runir::kr::dl::Register<Category>> reg_,
         IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects_,
         IndexList<runir::kr::ps::ext::OrderTerm> order_) :
        index(),
        source(std::move(source_)),
        target(std::move(target_)),
        conditions(std::move(conditions_)),
        feature(std::move(feature_)),
        reg(std::move(reg_)),
        effects(std::move(effects_)),
        order(std::move(order_))
    {
    }
    template<typename C, typename D>
    Data(::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> source_,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> target_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>>& conditions_,
         ::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, Category>>, C> feature_,
         ::ygg::View<Index<runir::kr::dl::Register<Category>>, D> reg_,
         const std::vector<::ygg::View<Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C>>& effects_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ext::OrderTerm>, C>>& order_) :
        index(),
        source(),
        target(),
        conditions(),
        feature(),
        reg(),
        effects(),
        order()
    {
        set(source_, source);
        set(target_, target);
        set(conditions_, conditions);
        set(feature_, feature);
        set(reg_, reg);
        set(effects_, effects);
        set(order_, order);
    }

    auto cista_members() noexcept { return std::tie(index, source, target, conditions, feature, reg, effects, order); }
    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, feature, reg, effects, order); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, feature, reg, effects, order); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects;

    Data() = default;
    Data(Index<runir::kr::ps::ext::MemoryState> source_,
         Index<runir::kr::ps::ext::MemoryState> target_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions_,
         IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects_) :
        index(),
        source(std::move(source_)),
        target(std::move(target_)),
        conditions(std::move(conditions_)),
        effects(std::move(effects_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> source_,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> target_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>>& conditions_,
         const std::vector<::ygg::View<Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C>>& effects_) :
        index(),
        source(),
        target(),
        conditions(),
        effects()
    {
        set(source_, source);
        set(target_, target);
        set(conditions_, conditions);
        set(effects_, effects);
    }

    auto cista_members() noexcept { return std::tie(index, source, target, conditions, effects); }
    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, effects); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, effects); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects;
    ::cista::offset::string action_name;
    IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>> arguments;

    Data() = default;
    Data(Index<runir::kr::ps::ext::MemoryState> source_,
         Index<runir::kr::ps::ext::MemoryState> target_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions_,
         IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects_,
         ::cista::offset::string action_name_,
         IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>> arguments_) :
        index(),
        source(std::move(source_)),
        target(std::move(target_)),
        conditions(std::move(conditions_)),
        effects(std::move(effects_)),
        action_name(std::move(action_name_)),
        arguments(std::move(arguments_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> source_,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> target_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>>& conditions_,
         const std::vector<::ygg::View<Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C>>& effects_,
         ::cista::offset::string action_name_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>, C>>& arguments_) :
        index(),
        source(),
        target(),
        conditions(),
        effects(),
        action_name(std::move(action_name_)),
        arguments()
    {
        set(source_, source);
        set(target_, target);
        set(conditions_, conditions);
        set(effects_, effects);
        set(arguments_, arguments);
    }

    auto cista_members() noexcept { return std::tie(index, source, target, conditions, effects, action_name, arguments); }
    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, effects, action_name, arguments); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, effects, action_name, arguments); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ActionTag>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ActionTag>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects;
    ::cista::offset::string action_name;
    Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>> query_feature;

    Data() = default;
    Data(Index<runir::kr::ps::ext::MemoryState> source_,
         Index<runir::kr::ps::ext::MemoryState> target_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions_,
         IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects_,
         ::cista::offset::string action_name_,
         Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>> query_feature_) :
        index(),
        source(std::move(source_)),
        target(std::move(target_)),
        conditions(std::move(conditions_)),
        effects(std::move(effects_)),
        action_name(std::move(action_name_)),
        query_feature(std::move(query_feature_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> source_,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> target_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>>& conditions_,
         const std::vector<::ygg::View<Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, C>>& effects_,
         ::cista::offset::string action_name_,
         ::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>, C> query_feature_) :
        index(),
        source(),
        target(),
        conditions(),
        effects(),
        action_name(std::move(action_name_)),
        query_feature()
    {
        set(source_, source);
        set(target_, target);
        set(conditions_, conditions);
        set(effects_, effects);
        set(query_feature_, query_feature);
    }

    auto cista_members() noexcept { return std::tie(index, source, target, conditions, effects, action_name, query_feature); }
    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, effects, action_name, query_feature); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, effects, action_name, query_feature); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    Index<runir::kr::ps::ext::ModuleSymbol> callee;
    ::cista::offset::vector<runir::kr::ps::ext::CallArgument> arguments;

    Data() = default;
    Data(Index<runir::kr::ps::ext::MemoryState> source_,
         Index<runir::kr::ps::ext::MemoryState> target_,
         IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions_,
         Index<runir::kr::ps::ext::ModuleSymbol> callee_,
         ::cista::offset::vector<runir::kr::ps::ext::CallArgument> arguments_) :
        index(),
        source(std::move(source_)),
        target(std::move(target_)),
        conditions(std::move(conditions_)),
        callee(std::move(callee_)),
        arguments(std::move(arguments_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> source_,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> target_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>>& conditions_,
         ::ygg::View<Index<runir::kr::ps::ext::ModuleSymbol>, C> callee_,
         ::cista::offset::vector<runir::kr::ps::ext::CallArgument> arguments_) :
        index(),
        source(),
        target(),
        conditions(),
        callee(),
        arguments(std::move(arguments_))
    {
        set(source_, source);
        set(target_, target);
        set(conditions_, conditions);
        set(callee_, callee);
    }

    auto cista_members() noexcept { return std::tie(index, source, target, conditions, callee, arguments); }
    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, callee, arguments); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, callee, arguments); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

template<>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::BacktrackTag>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::BacktrackTag>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;

    Data() = default;
    Data(Index<runir::kr::ps::ext::MemoryState> source_, IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions_) :
        index(),
        source(std::move(source_)),
        conditions(std::move(conditions_))
    {
    }
    template<typename C>
    Data(::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> source_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, C>>& conditions_) :
        index(),
        source(),
        conditions()
    {
        set(source_, source);
        set(conditions_, conditions);
    }

    auto cista_members() noexcept { return std::tie(index, source, conditions); }
    auto cista_members() const noexcept { return std::tie(index, source, conditions); }
    auto identifying_members() const noexcept { return std::tie(source, conditions); }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
