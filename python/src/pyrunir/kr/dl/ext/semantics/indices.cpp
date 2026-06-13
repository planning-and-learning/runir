#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/ext/declarations.hpp>
#include <runir/kr/dl/cnf_grammar/indices.hpp>
#include <runir/kr/dl/grammar/indices.hpp>
#include <runir/kr/dl/indices.hpp>
#include <runir/kr/dl/semantics/denotation_index.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::ext
{

void bind_semantics_indices(nb::module_& m)
{
    // Denotation indices are family-independent and registered once by the base semantics module.
    // The Constructor<ExtFamilyTag, Category> indices are registered by the ext dl module
    // (ConceptConstructorIndex etc. in kr/dl/ext/indices.cpp); both are intentionally not rebound here.

    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, BotTag>>>(m, "ConceptBotIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, TopTag>>>(m, "ConceptTopIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "ConceptAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "ConceptAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "ConceptAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, IntersectionTag>>>(m, "ConceptIntersectionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, UnionTag>>>(m, "ConceptUnionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, NegationTag>>>(m, "ConceptNegationIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, ExistentialQuantificationTag>>>(m, "ConceptExistentialQuantificationIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AtLeastNumberRestrictionTag>>>(m, "ConceptAtLeastNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AtMostNumberRestrictionTag>>>(m, "ConceptAtMostNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, ExactNumberRestrictionTag>>>(m, "ConceptExactNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, QualifiedAtLeastNumberRestrictionTag>>>(m, "ConceptQualifiedAtLeastNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, QualifiedAtMostNumberRestrictionTag>>>(m, "ConceptQualifiedAtMostNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, QualifiedExactNumberRestrictionTag>>>(m, "ConceptQualifiedExactNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, RoleValueMapTag>>>(m, "ConceptRoleValueMapIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, AgreementTag>>>(m, "ConceptAgreementIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, RoleFillersTag>>>(m, "ConceptRoleFillersIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, OneOfTag>>>(m, "ConceptOneOfIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, NominalTag>>>(m, "ConceptNominalIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, RegisterTag>>>(m, "ConceptRegisterIndex");
    ygg::bind_index<ygg::Index<Concept<runir::kr::ExtFamilyTag, ArgumentTag<ConceptTag>>>>(m, "ConceptArgumentIndex");

    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, UniversalTag>>>(m, "RoleUniversalIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "RoleAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "RoleAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "RoleAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, IntersectionTag>>>(m, "RoleIntersectionIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, UnionTag>>>(m, "RoleUnionIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, ComplementTag>>>(m, "RoleComplementIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, InverseTag>>>(m, "RoleInverseIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, CompositionTag>>>(m, "RoleCompositionIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, TransitiveClosureTag>>>(m, "RoleTransitiveClosureIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, ReflexiveTransitiveClosureTag>>>(m, "RoleReflexiveTransitiveClosureIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, RestrictionTag>>>(m, "RoleRestrictionIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, IdentityTag>>>(m, "RoleIdentityIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, RegisterTag>>>(m, "RoleRegisterIndex");
    ygg::bind_index<ygg::Index<Role<runir::kr::ExtFamilyTag, ArgumentTag<RoleTag>>>>(m, "RoleArgumentIndex");

    ygg::bind_index<ygg::Index<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(m, "BooleanAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(m, "BooleanAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(m, "BooleanAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::ExtFamilyTag, NonemptyTag>>>(m, "BooleanNonemptyIndex");
    ygg::bind_index<ygg::Index<Boolean<runir::kr::ExtFamilyTag, ArgumentTag<BooleanTag>>>>(m, "BooleanArgumentIndex");

    ygg::bind_index<ygg::Index<Numerical<runir::kr::ExtFamilyTag, CountTag>>>(m, "NumericalCountIndex");
    ygg::bind_index<ygg::Index<Numerical<runir::kr::ExtFamilyTag, DistanceTag>>>(m, "NumericalDistanceIndex");
    ygg::bind_index<ygg::Index<Numerical<runir::kr::ExtFamilyTag, ArgumentTag<NumericalTag>>>>(m, "NumericalArgumentIndex");

    // Register/argument identifiers referenced by the ext register/argument constructor data.
    // Concept and argument identifiers are already registered by the ext dl module, but the role
    // register identifier is reachable only through the role register data exposed here.
    ygg::bind_index<runir::kr::dl::RegisterIdentifier<runir::kr::dl::RoleTag>>(m, "RoleRegisterIdentifier");
}

}  // namespace runir::kr::dl::ext
