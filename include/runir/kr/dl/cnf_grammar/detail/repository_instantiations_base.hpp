#ifndef RUNIR_KR_DL_CNF_GRAMMAR_DETAIL_REPOSITORY_INSTANTIATIONS_BASE_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_DETAIL_REPOSITORY_INSTANTIATIONS_BASE_HPP_

/// Extern declarations of the symbol storage of every cnf base symbol type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::BotTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::TopTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::IntersectionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::UnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NegationTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ValueRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ExistentialQuantificationTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtLeastNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AtMostNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::ExactNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleValueMapTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::AgreementTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::RoleFillersTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::OneOfTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, runir::kr::dl::NominalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UniversalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IntersectionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::UnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::ComplementTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::InverseTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::CompositionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::TransitiveClosureTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::ReflexiveTransitiveClosureTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::RestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, runir::kr::dl::IdentityTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, runir::kr::dl::NonemptyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::CountTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::BaseFamilyTag, runir::kr::dl::DistanceTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::BaseFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::BaseFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Grammar<runir::kr::BaseFamilyTag>>;

#else

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

#endif

#endif
