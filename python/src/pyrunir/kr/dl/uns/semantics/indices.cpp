#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/uns/declarations.hpp>
#include <runir/kr/dl/uns/repository.hpp>
#include <runir/kr/dl/semantics/denotation_index.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::uns
{

void bind_semantics_indices(nb::module_& m)
{
    // Denotation indices and Constructor<runir::kr::UnsFamilyTag, Category> indices are registered elsewhere
    // (base semantics module and the uns dl module respectively); only the per-tag constructor
    // indices are introduced here.
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, BotTag>>>(m, "ConceptBotIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, TopTag>>>(m, "ConceptTopIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, IntersectionTag>>>(m, "ConceptIntersectionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, UnionTag>>>(m, "ConceptUnionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, NegationTag>>>(m, "ConceptNegationIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AtLeastNumberRestrictionTag>>>(m, "ConceptAtLeastNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AtMostNumberRestrictionTag>>>(m, "ConceptAtMostNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, ExactNumberRestrictionTag>>>(m, "ConceptExactNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, QualifiedAtLeastNumberRestrictionTag>>>(m, "ConceptQualifiedAtLeastNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, QualifiedAtMostNumberRestrictionTag>>>(m, "ConceptQualifiedAtMostNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, QualifiedExactNumberRestrictionTag>>>(m, "ConceptQualifiedExactNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, RoleValueMapTag>>>(m, "ConceptRoleValueMapIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, AgreementTag>>>(m, "ConceptAgreementIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, RoleFillersTag>>>(m, "ConceptRoleFillersIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, OneOfTag>>>(m, "ConceptOneOfIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::UnsFamilyTag, NominalTag>>>(m, "ConceptNominalIndex");

    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, UniversalTag>>>(m, "RoleUniversalIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, IntersectionTag>>>(m, "RoleIntersectionIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, UnionTag>>>(m, "RoleUnionIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, ComplementTag>>>(m, "RoleComplementIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, InverseTag>>>(m, "RoleInverseIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, CompositionTag>>>(m, "RoleCompositionIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, RestrictionTag>>>(m, "RoleRestrictionIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::UnsFamilyTag, IdentityTag>>>(m, "RoleIdentityIndex");

    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, NonemptyTag>>>(m, "BooleanNonemptyIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, EqTag<BooleanTag>>>>(m, "BooleanEqIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, NeqTag<BooleanTag>>>>(m, "BooleanNeqIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, LtTag<BooleanTag>>>>(m, "BooleanLtIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, LeTag<BooleanTag>>>>(m, "BooleanLeIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, GtTag<BooleanTag>>>>(m, "BooleanGtIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, GeTag<BooleanTag>>>>(m, "BooleanGeIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, EqTag<NumericalTag>>>>(m, "NumericalEqIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, NeqTag<NumericalTag>>>>(m, "NumericalNeqIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, LtTag<NumericalTag>>>>(m, "NumericalLtIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, LeTag<NumericalTag>>>>(m, "NumericalLeIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, GtTag<NumericalTag>>>>(m, "NumericalGtIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, GeTag<NumericalTag>>>>(m, "NumericalGeIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::UnsFamilyTag, BooleanConstantTag>>>(m, "BooleanConstantIndex");

    ygg::bind_index<ygg::Index<Numerical<runir::kr::UnsFamilyTag, CountTag>>>(m, "NumericalCountIndex");
    ygg::bind_index<ygg::Index<Numerical<runir::kr::UnsFamilyTag, DistanceTag>>>(m, "NumericalDistanceIndex");
    ygg::bind_index<ygg::Index<Numerical<runir::kr::UnsFamilyTag, NumericalConstantTag>>>(m, "NumericalConstantIndex");
}

}  // namespace runir::kr::dl::uns
