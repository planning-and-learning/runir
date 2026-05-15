#include "module.hpp"

#include <runir/kr/dl/grammar/indices.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_grammar_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<BotTag>>>(m, "ConceptBotIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<TopTag>>>(m, "ConceptTopIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<IntersectionTag>>>(m, "ConceptIntersectionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<UnionTag>>>(m, "ConceptUnionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<NegationTag>>>(m, "ConceptNegationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<RoleValueMapContainmentTag>>>(m, "ConceptRoleValueMapContainmentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<RoleValueMapEqualityTag>>>(m, "ConceptRoleValueMapEqualityIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Concept<NominalTag>>>(m, "ConceptNominalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<UniversalTag>>>(m, "RoleUniversalIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<IntersectionTag>>>(m, "RoleIntersectionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<UnionTag>>>(m, "RoleUnionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<ComplementTag>>>(m, "RoleComplementIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<InverseTag>>>(m, "RoleInverseIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<CompositionTag>>>(m, "RoleCompositionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<RestrictionTag>>>(m, "RoleRestrictionIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Role<IdentityTag>>>(m, "RoleIdentityIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Boolean<NonemptyTag>>>(m, "BooleanNonemptyIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Numerical<CountTag>>>(m, "NumericalCountIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Numerical<DistanceTag>>>(m, "NumericalDistanceIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Constructor<ConceptTag>>>(m, "ConceptIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Constructor<RoleTag>>>(m, "RoleIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Constructor<BooleanTag>>>(m, "BooleanIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::grammar::Constructor<NumericalTag>>>(m, "NumericalIndex");
}

}  // namespace runir::kr::dl
