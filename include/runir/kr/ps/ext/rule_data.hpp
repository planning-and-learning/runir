#ifndef RUNIR_KR_PS_EXT_RULE_DATA_HPP_
#define RUNIR_KR_PS_EXT_RULE_DATA_HPP_

#include "runir/kr/dl/constructor_index.hpp"
#include "runir/kr/dl/constructors.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/condition_index.hpp"
#include "runir/kr/ps/effect_index.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/module_index.hpp"
#include "runir/kr/ps/ext/register_index.hpp"
#include "runir/kr/ps/ext/rule_index.hpp"

#include <cista/containers/string.h>
#include <cista/containers/variant.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace runir::kr::ps::ext
{

using ConceptArgument = ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
using RoleArgument = ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
using BooleanArgument = ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>;
using NumericalArgument = ygg::Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>;
using CallArgument = ::cista::offset::variant<ConceptArgument, RoleArgument, BooleanArgument, NumericalArgument>;

}  // namespace runir::kr::ps::ext

namespace ygg
{

template<runir::kr::ps::ext::LoadCategory Category>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<Category>>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<Category>>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    Index<runir::kr::dl::FamilyConstructor<runir::kr::ExtFamilyTag, Category>> load_expression;
    Index<runir::kr::ps::ext::Register<Category>> reg;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(source);
        ygg::clear(target);
        ygg::clear(conditions);
        ygg::clear(load_expression);
        ygg::clear(reg);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, load_expression, reg); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, load_expression, reg); }
};

template<>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>> effects;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(source);
        ygg::clear(target);
        ygg::clear(conditions);
        ygg::clear(effects);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, effects); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, effects); }
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
    Data(::cista::offset::string action_name_) : index(), action_name(std::move(action_name_)) {}
    Data(const std::string& action_name_) : index(), action_name(action_name_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(source);
        ygg::clear(target);
        ygg::clear(conditions);
        ygg::clear(effects);
        ygg::clear(action_name);
        ygg::clear(arguments);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, effects, action_name, arguments); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, effects, action_name, arguments); }
};

template<>
struct Data<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>
{
    Index<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>> index;
    Index<runir::kr::ps::ext::MemoryState> source;
    Index<runir::kr::ps::ext::MemoryState> target;
    IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>> conditions;
    ::cista::offset::string callee_name;
    Index<runir::kr::ps::ext::Module> callee;
    ::cista::offset::vector<runir::kr::ps::ext::CallArgument> arguments;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(source);
        ygg::clear(target);
        ygg::clear(conditions);
        ygg::clear(callee_name);
        ygg::clear(callee);
        ygg::clear(arguments);
    }

    auto cista_members() const noexcept { return std::tie(index, source, target, conditions, callee_name, callee, arguments); }
    auto identifying_members() const noexcept { return std::tie(source, target, conditions, callee_name, callee, arguments); }
};

}  // namespace ygg

#endif
