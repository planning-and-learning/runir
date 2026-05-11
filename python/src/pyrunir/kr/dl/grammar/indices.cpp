#include "module.hpp"

#include <runir/kr/dl/grammar/indices.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

namespace grammar = runir::kr::dl::grammar;

void bind_grammar_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<grammar::Concept<BotTag>>>(m, "ConceptBotIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<TopTag>>>(m, "ConceptTopIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<IntersectionTag>>>(m, "ConceptIntersectionIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<UnionTag>>>(m, "ConceptUnionIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<NegationTag>>>(m, "ConceptNegationIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<RoleValueMapContainmentTag>>>(m, "ConceptRoleValueMapContainmentIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<RoleValueMapEqualityTag>>>(m, "ConceptRoleValueMapEqualityIndex");
    tyr::bind_index<tyr::Index<grammar::Concept<NominalTag>>>(m, "ConceptNominalIndex");
    tyr::bind_index<tyr::Index<grammar::Role<UniversalTag>>>(m, "RoleUniversalIndex");
    tyr::bind_index<tyr::Index<grammar::Role<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<grammar::Role<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<grammar::Role<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<grammar::Role<AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<grammar::Role<AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<grammar::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<grammar::Role<IntersectionTag>>>(m, "RoleIntersectionIndex");
    tyr::bind_index<tyr::Index<grammar::Role<UnionTag>>>(m, "RoleUnionIndex");
    tyr::bind_index<tyr::Index<grammar::Role<ComplementTag>>>(m, "RoleComplementIndex");
    tyr::bind_index<tyr::Index<grammar::Role<InverseTag>>>(m, "RoleInverseIndex");
    tyr::bind_index<tyr::Index<grammar::Role<CompositionTag>>>(m, "RoleCompositionIndex");
    tyr::bind_index<tyr::Index<grammar::Role<TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<grammar::Role<ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<grammar::Role<RestrictionTag>>>(m, "RoleRestrictionIndex");
    tyr::bind_index<tyr::Index<grammar::Role<IdentityTag>>>(m, "RoleIdentityIndex");
    tyr::bind_index<tyr::Index<grammar::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<grammar::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<grammar::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<grammar::Boolean<NonemptyTag>>>(m, "BooleanNonemptyIndex");
    tyr::bind_index<tyr::Index<grammar::Numerical<CountTag>>>(m, "NumericalCountIndex");
    tyr::bind_index<tyr::Index<grammar::Numerical<DistanceTag>>>(m, "NumericalDistanceIndex");
    tyr::bind_index<tyr::Index<grammar::Constructor<ConceptTag>>>(m, "ConceptIndex");
    tyr::bind_index<tyr::Index<grammar::Constructor<RoleTag>>>(m, "RoleIndex");
    tyr::bind_index<tyr::Index<grammar::Constructor<BooleanTag>>>(m, "BooleanIndex");
    tyr::bind_index<tyr::Index<grammar::Constructor<NumericalTag>>>(m, "NumericalIndex");
}

}  // namespace runir::kr::dl
