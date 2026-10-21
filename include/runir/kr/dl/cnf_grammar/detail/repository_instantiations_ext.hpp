#ifndef RUNIR_KR_DL_CNF_GRAMMAR_DETAIL_REPOSITORY_INSTANTIATIONS_EXT_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_DETAIL_REPOSITORY_INSTANTIATIONS_EXT_HPP_

/// Extern declarations of the symbol storage of every cnf ext symbol type, instantiated in the library.
/// Include only from sources that intern many of these types; see yggdrasil's docs/TEMPLATE_RULES.md.

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::BotTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::TopTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::IntersectionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::UnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::NegationTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ValueRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ExistentialQuantificationTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtLeastNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AtMostNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ExactNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::QualifiedExactNumberRestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RoleValueMapTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::AgreementTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RoleFillersTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::OneOfTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::NominalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::UniversalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::IntersectionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::UnionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ComplementTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::InverseTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::CompositionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::TransitiveClosureTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ReflexiveTransitiveClosureTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RestrictionTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::IdentityTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicStateTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::StaticTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::FluentTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NonemptyTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanEqTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanNeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanLtTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanLeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanGtTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanGeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalEqTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalNeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalLtTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalLeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalGtTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalGeTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanConstantTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::AndTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::OrTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Boolean<runir::kr::ExtFamilyTag, runir::kr::dl::NotTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::CountTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::DistanceTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalConstantTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::AddTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::SubTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::MulTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::DivTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::MinTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::MaxTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Numerical<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag> >>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Constructor<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::DerivationRule<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::SubstitutionRule<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>;
extern template class ygg::formalism::BasicSymbolRepository<runir::kr::dl::cnf_grammar::Grammar<runir::kr::ExtFamilyTag>>;

#else

#include <yggdrasil/formalism/detail/basic_symbol_repository_impl.hpp>

#endif

#endif
