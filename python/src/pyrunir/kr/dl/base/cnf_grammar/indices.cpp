#include "module.hpp"

#include <runir/kr/dl/cnf_grammar/indices.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::base
{

void bind_cnf_grammar_indices(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, BotTag>>>(m, "ConceptBotIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, TopTag>>>(m, "ConceptTopIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(
        m,
        "ConceptAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(
        m,
        "ConceptAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(
        m,
        "ConceptAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(
        m,
        "ConceptAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(
        m,
        "ConceptAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(
        m,
        "ConceptAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, IntersectionTag>>>(m, "ConceptIntersectionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, UnionTag>>>(m, "ConceptUnionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, NegationTag>>>(m, "ConceptNegationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ExistentialQuantificationTag>>>(
        m,
        "ConceptExistentialQuantificationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtLeastNumberRestrictionTag>>>(
        m,
        "ConceptAtLeastNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtMostNumberRestrictionTag>>>(
        m,
        "ConceptAtMostNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ExactNumberRestrictionTag>>>(m,
                                                                                                                          "ConceptExactNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>>>(
        m,
        "ConceptQualifiedAtLeastNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>>>(
        m,
        "ConceptQualifiedAtMostNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedExactNumberRestrictionTag>>>(
        m,
        "ConceptQualifiedExactNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, RoleValueMapTag>>>(m, "ConceptRoleValueMapIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AgreementTag>>>(m, "ConceptAgreementIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, RoleFillersTag>>>(m, "ConceptRoleFillersIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, OneOfTag>>>(m, "ConceptOneOfIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, NominalTag>>>(m, "ConceptNominalIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, UniversalTag>>>(m, "RoleUniversalIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(
        m,
        "RoleAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(
        m,
        "RoleAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(
        m,
        "RoleAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(
        m,
        "RoleAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(
        m,
        "RoleAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(
        m,
        "RoleAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, IntersectionTag>>>(m, "RoleIntersectionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, UnionTag>>>(m, "RoleUnionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, ComplementTag>>>(m, "RoleComplementIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, InverseTag>>>(m, "RoleInverseIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, CompositionTag>>>(m, "RoleCompositionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, ReflexiveTransitiveClosureTag>>>(
        m,
        "RoleReflexiveTransitiveClosureIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, RestrictionTag>>>(m, "RoleRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, IdentityTag>>>(m, "RoleIdentityIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(
        m,
        "BooleanAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(
        m,
        "BooleanAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(
        m,
        "BooleanAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(
        m,
        "BooleanAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(
        m,
        "BooleanAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(
        m,
        "BooleanAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, NonemptyTag>>>(m, "BooleanNonemptyIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Numerical<runir::kr::BaseFamilyTag, CountTag>>>(m, "NumericalCountIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Numerical<runir::kr::BaseFamilyTag, DistanceTag>>>(m, "NumericalDistanceIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, ConceptTag>>>(m, "ConceptIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, RoleTag>>>(m, "RoleIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, BooleanTag>>>(m, "BooleanIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, NumericalTag>>>(m, "NumericalIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, ConceptTag>>>(m, "ConceptNonTerminalIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, RoleTag>>>(m, "RoleNonTerminalIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, BooleanTag>>>(m, "BooleanNonTerminalIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, NumericalTag>>>(m, "NumericalNonTerminalIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Grammar<runir::kr::BaseFamilyTag>>>(m, "GrammarIndex");
}

}  // namespace runir::kr::dl::base
