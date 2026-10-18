#ifndef RUNIR_KR_DL_DETAIL_REPOSITORY_INSTANTIATIONS_BASE_HPP_
#define RUNIR_KR_DL_DETAIL_REPOSITORY_INSTANTIATIONS_BASE_HPP_

/// Extern declarations of the symbol storage of every dl base symbol type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/dl/repository.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::BotTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::TopTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::IntersectionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::UnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NegationTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ValueRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ExistentialQuantificationTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtLeastNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtMostNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ExactNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleValueMapTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AgreementTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleFillersTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::OneOfTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NominalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UniversalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IntersectionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::ComplementTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::InverseTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::CompositionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::TransitiveClosureTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::ReflexiveTransitiveClosureTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::RestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IdentityTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::DistanceTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryRoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryJoinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryProjectTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryRenameTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QuerySelectEqualTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QuerySelectValueTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryUnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryDifferenceTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryGenericJoinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryEmptyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, runir::kr::dl::QueryMustPassTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::BaseFamilyTag, void>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::QueryColumn>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::QueryProjection<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::QueryProjection<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>>;

#endif

#endif
