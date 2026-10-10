#ifndef RUNIR_KR_PS_ICP_DECLARATIONS_HPP_
#define RUNIR_KR_PS_ICP_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <concepts>
#include <memory>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

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

enum class ConditionOperation
{
    BELONGS,
    NOT_BELONGS
};

enum class EffectOperation
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

using LoadRuleTypes = ygg::TypeList<Rule<LoadTag<runir::kr::dl::ConceptTag>>, Rule<LoadTag<runir::kr::dl::RoleTag>>>;
using ConcreteRuleTypes = ygg::ConcatTypeListsT<LoadRuleTypes, ygg::TypeList<Rule<CruleTag>>>;
using RuleTypes = ygg::ConcatTypeListsT<ygg::TypeList<ps::Rule<IcpFamilyTag>>, ConcreteRuleTypes>;
using FeatureTypes = ps::PsFeatureTypes<IcpFamilyTag>;
using ConditionTypes = ps::PsConditionTypes<IcpFamilyTag>;
using EffectTypes = ps::PsEffectTypes<IcpFamilyTag>;
using ProgramTypes = ygg::TypeList<MemoryState, ModuleSymbol, Module, Program>;
using IndexicalTypes = ygg::TypeList<XCondition, XEffect>;
using RepositoryTypes = ygg::ConcatTypeListsT<ps::PsCoreTypes<runir::kr::IcpFamilyTag>, RuleTypes, ProgramTypes, IndexicalTypes>;
using Repository = ps::BasicRepository<IcpFamilyTag, RepositoryTypes>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory = ps::BasicRepositoryFactory<IcpFamilyTag, RepositoryTypes>;
using MemoryStateView = ygg::View<ygg::Index<MemoryState>, Repository>;
using ModuleSymbolView = ygg::View<ygg::Index<ModuleSymbol>, Repository>;
using ModuleView = ygg::View<ygg::Index<Module>, Repository>;
using ProgramView = ygg::View<ygg::Index<Program>, Repository>;
using XConditionView = ygg::View<ygg::Index<XCondition>, Repository>;
using XEffectView = ygg::View<ygg::Index<XEffect>, Repository>;
using RuleVariantView = ygg::View<ygg::Index<ps::Rule<IcpFamilyTag>>, Repository>;

template<RuleKind Kind>
using RuleView = ygg::View<ygg::Index<Rule<Kind>>, Repository>;

namespace detail
{
template<tyr::TaskKind Kind, RuleKind Tag>
class RuleEvaluator;
}

}

namespace runir::kr::ps::icp::dl
{

using BooleanFeatureView = ygg::View<ygg::Index<ps::Feature<IcpFamilyTag, runir::kr::dl::BooleanTag>>, Repository>;
using NumericalFeatureView = ygg::View<ygg::Index<ps::Feature<IcpFamilyTag, runir::kr::dl::NumericalTag>>, Repository>;
using QueryFeatureView = ygg::View<ygg::Index<ps::Feature<IcpFamilyTag, ps::dl::QueryFeature>>, Repository>;

}

#endif
