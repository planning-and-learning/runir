#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/indices.hpp>
#include <runir/kr/dl/grammar/indices.hpp>
#include <runir/kr/dl/indices.hpp>
#include <runir/kr/dl/semantics/denotation_index.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_semantics_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<runir::kr::dl::semantics::Denotation<BooleanTag>>>(m, "BooleanDenotationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::semantics::Denotation<NumericalTag>>>(m, "NumericalDenotationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::semantics::Denotation<ConceptTag>>>(m, "ConceptDenotationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::semantics::Denotation<RoleTag>>>(m, "RoleDenotationIndex");

    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, BotTag>>>(m, "ConceptBotIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, TopTag>>>(m, "ConceptTopIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, IntersectionTag>>>(m, "ConceptIntersectionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, UnionTag>>>(m, "ConceptUnionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, NegationTag>>>(m, "ConceptNegationIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AtLeastNumberRestrictionTag>>>(m, "ConceptAtLeastNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AtMostNumberRestrictionTag>>>(m, "ConceptAtMostNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, ExactNumberRestrictionTag>>>(m, "ConceptExactNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>>>(m, "ConceptQualifiedAtLeastNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>>>(m, "ConceptQualifiedAtMostNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, QualifiedExactNumberRestrictionTag>>>(m, "ConceptQualifiedExactNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, RoleValueMapTag>>>(m, "ConceptRoleValueMapIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, AgreementTag>>>(m, "ConceptAgreementIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, RoleFillersTag>>>(m, "ConceptRoleFillersIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, OneOfTag>>>(m, "ConceptOneOfIndex");
    tyr::bind_index<tyr::Index<Concept<BaseFamilyTag, NominalTag>>>(m, "ConceptNominalIndex");

    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, UniversalTag>>>(m, "RoleUniversalIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, IntersectionTag>>>(m, "RoleIntersectionIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, UnionTag>>>(m, "RoleUnionIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, ComplementTag>>>(m, "RoleComplementIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, InverseTag>>>(m, "RoleInverseIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, CompositionTag>>>(m, "RoleCompositionIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, RestrictionTag>>>(m, "RoleRestrictionIndex");
    tyr::bind_index<tyr::Index<Role<BaseFamilyTag, IdentityTag>>>(m, "RoleIdentityIndex");

    tyr::bind_index<tyr::Index<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<Boolean<BaseFamilyTag, NonemptyTag>>>(m, "BooleanNonemptyIndex");

    tyr::bind_index<tyr::Index<Numerical<BaseFamilyTag, CountTag>>>(m, "NumericalCountIndex");
    tyr::bind_index<tyr::Index<Numerical<BaseFamilyTag, DistanceTag>>>(m, "NumericalDistanceIndex");

    tyr::bind_index<tyr::Index<Constructor<BaseFamilyTag, ConceptTag>>>(m, "ConceptIndex");
    tyr::bind_index<tyr::Index<Constructor<BaseFamilyTag, RoleTag>>>(m, "RoleIndex");
    tyr::bind_index<tyr::Index<Constructor<BaseFamilyTag, BooleanTag>>>(m, "BooleanIndex");
    tyr::bind_index<tyr::Index<Constructor<BaseFamilyTag, NumericalTag>>>(m, "NumericalIndex");
}

}  // namespace runir::kr::dl
