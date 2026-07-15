#ifndef RUNIR_KR_PS_EXT_DECLARATIONS_HPP_
#define RUNIR_KR_PS_EXT_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <concepts>
#include <memory>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

struct MemoryState
{
};

struct ModuleSymbol
{
};

// Rules

struct SketchTag
{
    static constexpr auto keyword = "sketch";
};

template<runir::kr::dl::CategoryTag CategoryT>
struct LoadTag
{
    static constexpr auto keyword = "load";
};

struct DoTag
{
    static constexpr auto keyword = "do";
};

struct CallTag
{
    static constexpr auto keyword = "call";
};

template<typename T>
concept RuleKind = std::same_as<T, LoadTag<runir::kr::dl::ConceptTag>> || std::same_as<T, LoadTag<runir::kr::dl::RoleTag>> || std::same_as<T, SketchTag>
                   || std::same_as<T, DoTag> || std::same_as<T, CallTag>;

template<RuleKind Kind>
struct Rule
{
};

struct RuleVariant
{
};

struct Module
{
};

struct ModuleProgram
{
};

using LoadRuleTypes = ygg::TypeList<Rule<LoadTag<runir::kr::dl::ConceptTag>>, Rule<LoadTag<runir::kr::dl::RoleTag>>>;
using ControlRuleTypes = ygg::MapTypeListT<Rule, ygg::TypeList<SketchTag, DoTag, CallTag>>;
using ConcreteRuleTypes = ygg::ConcatTypeListsT<LoadRuleTypes, ControlRuleTypes>;
using RuleTypes = ygg::ConcatTypeListsT<ygg::TypeList<RuleVariant>, ConcreteRuleTypes>;
using FeatureTypes = runir::kr::ps::PsFeatureTypes<runir::kr::ExtFamilyTag>;
using ConditionTypes = runir::kr::ps::PsConditionTypes<runir::kr::ExtFamilyTag>;
using EffectTypes = runir::kr::ps::PsEffectTypes<runir::kr::ExtFamilyTag>;
using ProgramTypes = ygg::TypeList<MemoryState, ModuleSymbol, Module, ModuleProgram>;
using RepositoryTypes = ygg::ConcatTypeListsT<runir::kr::ps::base::RepositoryTypes, FeatureTypes, ConditionTypes, EffectTypes, RuleTypes, ProgramTypes>;
using Repository =
    runir::kr::ps::BasicRepository<runir::kr::ExtFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::ExtFamilyTag>>;
using RepositoryPtr = std::shared_ptr<Repository>;
using RepositoryFactory =
    runir::kr::ps::BasicRepositoryFactory<runir::kr::ExtFamilyTag, RepositoryTypes, runir::kr::dl::ConstructorRepositoryPtrFor<runir::kr::ExtFamilyTag>>;

using ConceptRegisterView = ygg::View<ygg::Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>, runir::kr::dl::ExtConstructorRepository>;
using RoleRegisterView = ygg::View<ygg::Index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>, runir::kr::dl::ExtConstructorRepository>;
using MemoryStateView = ygg::View<ygg::Index<MemoryState>, Repository>;
using ModuleSymbolView = ygg::View<ygg::Index<ModuleSymbol>, Repository>;
using ModuleView = ygg::View<ygg::Index<Module>, Repository>;
using ModuleProgramView = ygg::View<ygg::Index<ModuleProgram>, Repository>;
using RuleVariantView = ygg::View<ygg::Index<RuleVariant>, Repository>;

template<RuleKind Kind>
using RuleView = ygg::View<ygg::Index<Rule<Kind>>, Repository>;

template<typename R>
concept LoadRuleView = std::same_as<R, RuleView<LoadTag<runir::kr::dl::ConceptTag>>> || std::same_as<R, RuleView<LoadTag<runir::kr::dl::RoleTag>>>;

}  // namespace runir::kr::ps::ext

namespace runir::kr::ps::ext::dl
{

using BooleanFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>, Repository>;
using NumericalFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>, Repository>;

}  // namespace runir::kr::ps::ext::dl

#endif
