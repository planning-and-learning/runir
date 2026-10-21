#ifndef RUNIR_KR_PS_EXT_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of the symbol storage of every ps ext symbol type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/ps/ext/repository.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::base::Sketch>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::QueryFeature>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ChooseTag<runir::kr::dl::ConceptTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ChooseTag<runir::kr::dl::RoleTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ActionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::BacktrackTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::MemoryState>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::ModuleSymbol>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Module>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Program>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::OrderTerm>;

#else

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

#endif

#endif
