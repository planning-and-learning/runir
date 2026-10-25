#include "runir/kr/ps/base/repository.hpp"

#include "runir/kr/ps/detail/repository_impl.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/icp/repository.hpp"
#include "runir/kr/uns/repository.hpp"

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::base::Sketch>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::QueryFeature>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<runir::kr::dl::ConceptTag>>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::LoadTag<runir::kr::dl::RoleTag>>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::SketchTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::DoTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::CallTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ChooseTag<runir::kr::dl::ConceptTag>>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ChooseTag<runir::kr::dl::RoleTag>>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::ActionTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Rule<runir::kr::ps::ext::BacktrackTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::MemoryState>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::ModuleSymbol>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Module>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::Program>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ext::OrderTerm>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::NumericalTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteFeature<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::QueryFeature>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteConditionVariant<runir::kr::IcpFamilyTag, runir::kr::DlTag>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::EqualZero>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteCondition<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::GreaterZero>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteEffectVariant<runir::kr::IcpFamilyTag, runir::kr::DlTag>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Positive>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Negative>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag, runir::kr::ps::dl::Unchanged>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Increases>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Decreases>>;
template class ygg::formalism::BasicSymbolRepository<
    runir::kr::ps::ConcreteEffect<runir::kr::IcpFamilyTag, runir::kr::DlTag, runir::kr::dl::NumericalTag, runir::kr::ps::dl::Unchanged>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<runir::kr::dl::ConceptTag>>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Rule<runir::kr::ps::icp::LoadTag<runir::kr::dl::RoleTag>>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Rule<runir::kr::ps::icp::CruleTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::MemoryState>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::ModuleSymbol>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Module>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::Program>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::XCondition>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::icp::XEffect>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::dl::BooleanTag>>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::uns::ClassifierLiteral>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::uns::ClassifierClause>;
template class ygg::formalism::BasicSymbolRepository<runir::kr::uns::Classifier>;

template class runir::kr::ps::BasicRepositoryFactory<runir::kr::BaseFamilyTag, runir::kr::ps::base::RepositoryTypes>;
template class runir::kr::ps::BasicRepositoryFactory<runir::kr::ExtFamilyTag, runir::kr::ps::ext::RepositoryTypes>;
template class runir::kr::ps::BasicRepositoryFactory<runir::kr::IcpFamilyTag, runir::kr::ps::icp::RepositoryTypes>;
template class runir::kr::ps::BasicRepositoryFactory<runir::kr::UnsFamilyTag, runir::kr::uns::RepositoryTypes>;
