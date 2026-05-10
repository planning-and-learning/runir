#include "module.hpp"

#include <runir/knowledge_representation/dl/cnf_grammar/indices.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

namespace cnf = runir::kr::dl::cnf_grammar;

void bind_cnf_grammar_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<cnf::Concept<BotTag>>>(m, "ConceptBotIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<TopTag>>>(m, "ConceptTopIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<IntersectionTag>>>(m, "ConceptIntersectionIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<UnionTag>>>(m, "ConceptUnionIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<NegationTag>>>(m, "ConceptNegationIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<RoleValueMapContainmentTag>>>(m, "ConceptRoleValueMapContainmentIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<RoleValueMapEqualityTag>>>(m, "ConceptRoleValueMapEqualityIndex");
    tyr::bind_index<tyr::Index<cnf::Concept<NominalTag>>>(m, "ConceptNominalIndex");
    tyr::bind_index<tyr::Index<cnf::Role<UniversalTag>>>(m, "RoleUniversalIndex");
    tyr::bind_index<tyr::Index<cnf::Role<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<cnf::Role<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<cnf::Role<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<cnf::Role<AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<cnf::Role<AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<cnf::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<cnf::Role<IntersectionTag>>>(m, "RoleIntersectionIndex");
    tyr::bind_index<tyr::Index<cnf::Role<UnionTag>>>(m, "RoleUnionIndex");
    tyr::bind_index<tyr::Index<cnf::Role<ComplementTag>>>(m, "RoleComplementIndex");
    tyr::bind_index<tyr::Index<cnf::Role<InverseTag>>>(m, "RoleInverseIndex");
    tyr::bind_index<tyr::Index<cnf::Role<CompositionTag>>>(m, "RoleCompositionIndex");
    tyr::bind_index<tyr::Index<cnf::Role<TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<cnf::Role<ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<cnf::Role<RestrictionTag>>>(m, "RoleRestrictionIndex");
    tyr::bind_index<tyr::Index<cnf::Role<IdentityTag>>>(m, "RoleIdentityIndex");
    tyr::bind_index<tyr::Index<cnf::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<cnf::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<cnf::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<cnf::Boolean<NonemptyTag>>>(m, "BooleanNonemptyIndex");
    tyr::bind_index<tyr::Index<cnf::Numerical<CountTag>>>(m, "NumericalCountIndex");
    tyr::bind_index<tyr::Index<cnf::Numerical<DistanceTag>>>(m, "NumericalDistanceIndex");
    tyr::bind_index<tyr::Index<cnf::Constructor<ConceptTag>>>(m, "ConceptIndex");
    tyr::bind_index<tyr::Index<cnf::Constructor<RoleTag>>>(m, "RoleIndex");
    tyr::bind_index<tyr::Index<cnf::Constructor<BooleanTag>>>(m, "BooleanIndex");
    tyr::bind_index<tyr::Index<cnf::Constructor<NumericalTag>>>(m, "NumericalIndex");
}

}  // namespace runir::kr::dl
