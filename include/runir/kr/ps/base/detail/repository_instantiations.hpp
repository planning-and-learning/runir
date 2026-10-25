#ifndef RUNIR_KR_PS_BASE_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of the symbol storage of every ps base symbol type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/ps/base/repository.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::base::Sketch>;

#else

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

#endif

#endif
