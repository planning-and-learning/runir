#ifndef RUNIR_KR_PS_ICP_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of the symbol storage of every ps icp symbol type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/ps/icp/repository.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::QueryFeature>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteConditionVariant<runir::kr::IcpFamilyTag, runir::kr::DlTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffectVariant<runir::kr::IcpFamilyTag, runir::kr::DlTag>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>>;
extern template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<runir::kr::dl::ConceptTag>>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<runir::kr::dl::RoleTag>>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::MemoryState>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::ModuleSymbol>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Module>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Program>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::XCondition>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::XEffect>;

#else

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

#endif

#endif
