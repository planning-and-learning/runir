#include "../module.hpp"
#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/formatter.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/python/bindings.hpp>

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

    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, BotTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, TopTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AtomicStateTag<StaticTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AtomicStateTag<FluentTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AtomicStateTag<DerivedTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AtomicGoalTag<StaticTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AtomicGoalTag<FluentTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AtomicGoalTag<DerivedTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, IntersectionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, UnionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, NegationTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, ValueRestrictionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, ExistentialQuantificationTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AtLeastNumberRestrictionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AtMostNumberRestrictionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, ExactNumberRestrictionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, QualifiedAtLeastNumberRestrictionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, QualifiedAtMostNumberRestrictionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, QualifiedExactNumberRestrictionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, RoleValueMapTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, AgreementTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, RoleFillersTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, OneOfTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, NominalTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, RegisterTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<SemanticFamily, ArgumentTag<ConceptTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, UniversalTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, AtomicStateTag<StaticTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, AtomicStateTag<FluentTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, AtomicStateTag<DerivedTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, AtomicGoalTag<StaticTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, AtomicGoalTag<FluentTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, AtomicGoalTag<DerivedTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, IntersectionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, UnionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, ComplementTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, InverseTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, CompositionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, TransitiveClosureTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, ReflexiveTransitiveClosureTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, RestrictionTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, IdentityTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, RegisterTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<SemanticFamily, ArgumentTag<RoleTag>>, ConstructorRepository>>(m);
}

}  // namespace runir::kr::dl
