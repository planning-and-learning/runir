#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/base/declarations.hpp>
#include <runir/kr/dl/cnf_grammar/indices.hpp>
#include <runir/kr/dl/grammar/indices.hpp>
#include <runir/kr/dl/indices.hpp>
#include <runir/kr/dl/semantics/denotation_index.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl::base
{

void bind_semantics_indices(nb::module_& m)
{
    tyr::bind_index<tyr::Index<runir::kr::dl::semantics::Denotation<BooleanTag>>>(m, "BooleanDenotationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::semantics::Denotation<NumericalTag>>>(m, "NumericalDenotationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::semantics::Denotation<ConceptTag>>>(m, "ConceptDenotationIndex");
    tyr::bind_index<tyr::Index<runir::kr::dl::semantics::Denotation<RoleTag>>>(m, "RoleDenotationIndex");

    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, BotTag>>>(m, "ConceptBotIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, TopTag>>>(m, "ConceptTopIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, IntersectionTag>>>(m, "ConceptIntersectionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, UnionTag>>>(m, "ConceptUnionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, NegationTag>>>(m, "ConceptNegationIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AtLeastNumberRestrictionTag>>>(m, "ConceptAtLeastNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AtMostNumberRestrictionTag>>>(m, "ConceptAtMostNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, ExactNumberRestrictionTag>>>(m, "ConceptExactNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>>>(m, "ConceptQualifiedAtLeastNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>>>(m, "ConceptQualifiedAtMostNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, QualifiedExactNumberRestrictionTag>>>(m, "ConceptQualifiedExactNumberRestrictionIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, RoleValueMapTag>>>(m, "ConceptRoleValueMapIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, AgreementTag>>>(m, "ConceptAgreementIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, RoleFillersTag>>>(m, "ConceptRoleFillersIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, OneOfTag>>>(m, "ConceptOneOfIndex");
    tyr::bind_index<tyr::Index<Concept<runir::kr::BaseFamilyTag, NominalTag>>>(m, "ConceptNominalIndex");

    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, UniversalTag>>>(m, "RoleUniversalIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, IntersectionTag>>>(m, "RoleIntersectionIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, UnionTag>>>(m, "RoleUnionIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, ComplementTag>>>(m, "RoleComplementIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, InverseTag>>>(m, "RoleInverseIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, CompositionTag>>>(m, "RoleCompositionIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, RestrictionTag>>>(m, "RoleRestrictionIndex");
    tyr::bind_index<tyr::Index<Role<runir::kr::BaseFamilyTag, IdentityTag>>>(m, "RoleIdentityIndex");

    tyr::bind_index<tyr::Index<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    tyr::bind_index<tyr::Index<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    tyr::bind_index<tyr::Index<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    tyr::bind_index<tyr::Index<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicGoalStaticIndex");
    tyr::bind_index<tyr::Index<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicGoalFluentIndex");
    tyr::bind_index<tyr::Index<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicGoalDerivedIndex");
    tyr::bind_index<tyr::Index<Boolean<runir::kr::BaseFamilyTag, NonemptyTag>>>(m, "BooleanNonemptyIndex");

    tyr::bind_index<tyr::Index<Numerical<runir::kr::BaseFamilyTag, CountTag>>>(m, "NumericalCountIndex");
    tyr::bind_index<tyr::Index<Numerical<runir::kr::BaseFamilyTag, DistanceTag>>>(m, "NumericalDistanceIndex");

    tyr::bind_index<tyr::Index<Constructor<runir::kr::BaseFamilyTag, ConceptTag>>>(m, "ConceptIndex");
    tyr::bind_index<tyr::Index<Constructor<runir::kr::BaseFamilyTag, RoleTag>>>(m, "RoleIndex");
    tyr::bind_index<tyr::Index<Constructor<runir::kr::BaseFamilyTag, BooleanTag>>>(m, "BooleanIndex");
    tyr::bind_index<tyr::Index<Constructor<runir::kr::BaseFamilyTag, NumericalTag>>>(m, "NumericalIndex");
}

}  // namespace runir::kr::dl::base
