#include "../record_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_semantics_records(nb::module_& m)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    urd::python::bind_data<Concept<SemanticFamily, BotTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, TopTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Concept<SemanticFamily, IntersectionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, UnionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, NegationTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, ValueRestrictionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, ExistentialQuantificationTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AtLeastNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AtMostNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, ExactNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, QualifiedAtLeastNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, QualifiedAtMostNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, QualifiedExactNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, RoleValueMapTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, AgreementTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, RoleFillersTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, OneOfTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, NominalTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, RegisterTag>>(m);
    urd::python::bind_data<Concept<SemanticFamily, ArgumentTag<ConceptTag>>>(m);
    urd::python::bind_data<Role<SemanticFamily, UniversalTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Role<SemanticFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Role<SemanticFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Role<SemanticFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Role<SemanticFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Role<SemanticFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Role<SemanticFamily, IntersectionTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, UnionTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, ComplementTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, InverseTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, CompositionTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, TransitiveClosureTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, ReflexiveTransitiveClosureTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, RestrictionTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, IdentityTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, RegisterTag>>(m);
    urd::python::bind_data<Role<SemanticFamily, ArgumentTag<RoleTag>>>(m);

    python::bind_record_views<SemanticFamily, ConstructorRepository>(m, ConceptTag {});
    python::bind_record_views<SemanticFamily, ConstructorRepository>(m, RoleTag {});
}

}  // namespace runir::kr::dl
