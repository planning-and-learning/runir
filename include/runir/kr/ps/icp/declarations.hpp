#ifndef RUNIR_KR_PS_ICP_DECLARATIONS_HPP_
#define RUNIR_KR_PS_ICP_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <concepts>
#include <cstdint>
#include <memory>
#include <tyr/planning/declarations.hpp>
#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>

namespace runir::kr::ps::icp
{

struct MemoryState
{
};

struct ModuleSymbol
{
};

struct Module
{
};

struct Program
{
};

struct XCondition
{
};

struct XEffect
{
};

enum class ConditionOperation : std::uint8_t
{
    BELONGS,
    NOT_BELONGS
};

enum class EffectOperation : std::uint8_t
{
    ENTER,
    EXIT
};

template<runir::kr::dl::CategoryTag CategoryT>
struct LoadTag
{
    using Category = CategoryT;
    static constexpr auto keyword = "load";
};

struct CruleTag
{
    static constexpr auto keyword = "crule";
};

template<typename T>
concept BindingRuleKind = std::same_as<T, LoadTag<runir::kr::dl::ConceptTag>> || std::same_as<T, LoadTag<runir::kr::dl::RoleTag>>;

template<typename T>
concept RuleKind = BindingRuleKind<T> || std::same_as<T, CruleTag>;

/// DL category bound by a rule tag.
template<BindingRuleKind Tag>
using RuleCategoryFor = typename Tag::Category;

template<RuleKind Kind>
struct Rule
{
};

using LoadRuleTypes = boost::mp11::mp_list<Rule<LoadTag<runir::kr::dl::ConceptTag>>, Rule<LoadTag<runir::kr::dl::RoleTag>>>;
using ConcreteRuleTypes = boost::mp11::mp_append<LoadRuleTypes, boost::mp11::mp_list<Rule<CruleTag>>>;
using RuleTypes = boost::mp11::mp_append<boost::mp11::mp_list<ps::Rule<IcpFamilyTag>>, ConcreteRuleTypes>;
using FeatureTypes = ps::PsFeatureTypes<IcpFamilyTag>;
using ConditionTypes = ps::PsConditionTypes<IcpFamilyTag>;
using EffectTypes = ps::PsEffectTypes<IcpFamilyTag>;
using ProgramTypes = boost::mp11::mp_list<MemoryState, ModuleSymbol, Module, Program>;
using IndexicalTypes = boost::mp11::mp_list<XCondition, XEffect>;
using RepositoryTypes = boost::mp11::mp_append<ps::PsCoreTypes<runir::kr::IcpFamilyTag>, RuleTypes, ProgramTypes, IndexicalTypes>;
using Repository = ps::BasicRepository<IcpFamilyTag, RepositoryTypes>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = ps::BasicRepositoryFactory<IcpFamilyTag, RepositoryTypes>;
using MemoryStateView = urd::view<MemoryState, Repository>;
using ModuleSymbolView = urd::view<ModuleSymbol, Repository>;
using ModuleView = urd::view<Module, Repository>;
using ProgramView = urd::view<Program, Repository>;
using XConditionView = urd::view<XCondition, Repository>;
using XEffectView = urd::view<XEffect, Repository>;
using RuleVariantView = urd::view<ps::Rule<IcpFamilyTag>, Repository>;

template<RuleKind Kind>
using RuleView = urd::view<Rule<Kind>, Repository>;

namespace detail
{
template<tyr::TaskKind Kind, RuleKind Tag>
class RuleEvaluator;
}

}

namespace runir::kr::ps::icp::dl
{

using BooleanFeatureView = urd::view<ps::Feature<IcpFamilyTag, runir::kr::dl::BooleanTag>, Repository>;
using NumericalFeatureView = urd::view<ps::Feature<IcpFamilyTag, runir::kr::dl::NumericalTag>, Repository>;
using QueryFeatureView = urd::view<ps::Feature<IcpFamilyTag, ps::dl::QueryFeature>, Repository>;

}

#endif
