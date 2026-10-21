#ifndef RUNIR_KR_DL_DETAIL_REPOSITORY_INSTANTIATIONS_UNS_HPP_
#define RUNIR_KR_DL_DETAIL_REPOSITORY_INSTANTIATIONS_UNS_HPP_

/// Extern declarations of the symbol storage of every dl uns symbol type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/dl/repository.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::BotTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::TopTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::IntersectionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::UnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NegationTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::ValueRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::ExistentialQuantificationTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtLeastNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AtMostNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::ExactNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::RoleValueMapTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::AgreementTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::RoleFillersTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::OneOfTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Concept<runir::kr::UnsFamilyTag, runir::kr::dl::NominalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::UniversalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::IntersectionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::UnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::ComplementTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::InverseTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::CompositionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::TransitiveClosureTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::ReflexiveTransitiveClosureTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::RestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Role<runir::kr::UnsFamilyTag, runir::kr::dl::IdentityTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NonemptyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanEqTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanNeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanLtTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanLeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanGtTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanGeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalEqTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalNeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalLtTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalLeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalGtTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalGeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanConstantTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::AndTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::OrTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Boolean<runir::kr::UnsFamilyTag, runir::kr::dl::NotTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::CountTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::DistanceTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalConstantTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::AddTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::SubTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::MulTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::DivTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::MinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Numerical<runir::kr::UnsFamilyTag, runir::kr::dl::MaxTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryRoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryJoinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryProjectTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryRenameTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QuerySelectEqualTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QuerySelectValueTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryUnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryDifferenceTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryGenericJoinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryEmptyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, runir::kr::dl::QueryMustPassTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::Query<runir::kr::UnsFamilyTag, void>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::QueryColumn>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::QueryProjection<runir::kr::UnsFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::QueryProjection<runir::kr::UnsFamilyTag, runir::kr::dl::RoleTag>>;

#else

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

#endif

#endif
