#include "module.hpp"

#include <runir/kr/dl/grammar/indices.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_grammar_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, BotTag>>>(m, "ConceptBotIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, TopTag>>>(m, "ConceptTopIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, IntersectionTag>>>(m, "ConceptIntersectionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, UnionTag>>>(m, "ConceptUnionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, NegationTag>>>(m, "ConceptNegationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtLeastNumberRestrictionTag>>>(m, "ConceptAtLeastNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtMostNumberRestrictionTag>>>(m, "ConceptAtMostNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, ExactNumberRestrictionTag>>>(m, "ConceptExactNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>>>(m, "ConceptQualifiedAtLeastNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>>>(m, "ConceptQualifiedAtMostNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, QualifiedExactNumberRestrictionTag>>>(m, "ConceptQualifiedExactNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, RoleValueMapTag>>>(m, "ConceptRoleValueMapIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AgreementTag>>>(m, "ConceptAgreementIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, RoleFillersTag>>>(m, "ConceptRoleFillersIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, OneOfTag>>>(m, "ConceptOneOfIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, NominalTag>>>(m, "ConceptNominalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, UniversalTag>>>(m, "RoleUniversalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, IntersectionTag>>>(m, "RoleIntersectionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, UnionTag>>>(m, "RoleUnionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, ComplementTag>>>(m, "RoleComplementIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, InverseTag>>>(m, "RoleInverseIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, CompositionTag>>>(m, "RoleCompositionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, RestrictionTag>>>(m, "RoleRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, IdentityTag>>>(m, "RoleIdentityIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, NonemptyTag>>>(m, "BooleanNonemptyIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Numerical<runir::kr::dl::BaseFamilyTag, CountTag>>>(m, "NumericalCountIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Numerical<runir::kr::dl::BaseFamilyTag, DistanceTag>>>(m, "NumericalDistanceIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Constructor<runir::kr::dl::BaseFamilyTag, ConceptTag>>>(m, "ConceptIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Constructor<runir::kr::dl::BaseFamilyTag, RoleTag>>>(m, "RoleIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Constructor<runir::kr::dl::BaseFamilyTag, BooleanTag>>>(m, "BooleanIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Constructor<runir::kr::dl::BaseFamilyTag, NumericalTag>>>(m, "NumericalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BaseFamilyTag, ConceptTag>>>(m, "ConceptNonTerminalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BaseFamilyTag, RoleTag>>>(m, "RoleNonTerminalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BaseFamilyTag, BooleanTag>>>(m, "BooleanNonTerminalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BaseFamilyTag, NumericalTag>>>(m, "NumericalNonTerminalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, ConceptTag>>>(m, "ConceptConstructorOrNonTerminalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, RoleTag>>>(m, "RoleConstructorOrNonTerminalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, BooleanTag>>>(m, "BooleanConstructorOrNonTerminalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, NumericalTag>>>(m, "NumericalConstructorOrNonTerminalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::GrammarTag<runir::kr::dl::BaseFamilyTag>>>(m, "GrammarIndex");
}

}  // namespace runir::kr::dl
