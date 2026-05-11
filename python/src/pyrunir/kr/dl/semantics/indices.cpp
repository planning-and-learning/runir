#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/indices.hpp>
#include <runir/kr/dl/grammar/indices.hpp>
#include <runir/kr/dl/indices.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_semantics_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<Concept<BotTag>>>(m, "ConceptBotIndex");
    tyr::bind_index<tyr::Index<Concept<TopTag>>>(m, "ConceptTopIndex");
    tyr::bind_index<tyr::Index<Concept<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Concept<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Concept<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Concept<AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<Concept<AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<Concept<IntersectionTag>>>(m, "ConceptIntersectionIndex");
    tyr::bind_index<tyr::Index<Concept<UnionTag>>>(m, "ConceptUnionIndex");
    tyr::bind_index<tyr::Index<Concept<NegationTag>>>(m, "ConceptNegationIndex");
    tyr::bind_index<tyr::Index<Concept<ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    tyr::bind_index<tyr::Index<Concept<RoleValueMapContainmentTag>>>(m, "ConceptRoleValueMapContainmentIndex");
    tyr::bind_index<tyr::Index<Concept<RoleValueMapEqualityTag>>>(m, "ConceptRoleValueMapEqualityIndex");
    tyr::bind_index<tyr::Index<Concept<NominalTag>>>(m, "ConceptNominalIndex");

    tyr::bind_index<tyr::Index<Role<UniversalTag>>>(m, "RoleUniversalIndex");
    tyr::bind_index<tyr::Index<Role<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Role<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Role<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Role<AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<Role<AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<Role<AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<Role<IntersectionTag>>>(m, "RoleIntersectionIndex");
    tyr::bind_index<tyr::Index<Role<UnionTag>>>(m, "RoleUnionIndex");
    tyr::bind_index<tyr::Index<Role<ComplementTag>>>(m, "RoleComplementIndex");
    tyr::bind_index<tyr::Index<Role<InverseTag>>>(m, "RoleInverseIndex");
    tyr::bind_index<tyr::Index<Role<CompositionTag>>>(m, "RoleCompositionIndex");
    tyr::bind_index<tyr::Index<Role<TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<Role<ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<Role<RestrictionTag>>>(m, "RoleRestrictionIndex");
    tyr::bind_index<tyr::Index<Role<IdentityTag>>>(m, "RoleIdentityIndex");

    tyr::bind_index<tyr::Index<Boolean<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Boolean<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Boolean<NonemptyTag>>>(m, "BooleanNonemptyIndex");

    tyr::bind_index<tyr::Index<Numerical<CountTag>>>(m, "NumericalCountIndex");
    tyr::bind_index<tyr::Index<Numerical<DistanceTag>>>(m, "NumericalDistanceIndex");

    tyr::bind_index<tyr::Index<Constructor<ConceptTag>>>(m, "ConceptIndex");
    tyr::bind_index<tyr::Index<Constructor<RoleTag>>>(m, "RoleIndex");
    tyr::bind_index<tyr::Index<Constructor<BooleanTag>>>(m, "BooleanIndex");
    tyr::bind_index<tyr::Index<Constructor<NumericalTag>>>(m, "NumericalIndex");
}

}  // namespace runir::kr::dl
