#include "../module.hpp"
#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/formatter.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <tyr/formalism/repository.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_grammar_records(nb::module_& m)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    urd::python::bind_index<Concept<grammar::GrammarFamily, BotTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, BotTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, TopTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, TopTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, IntersectionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, IntersectionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, UnionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, UnionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, NegationTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, NegationTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, ValueRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, ValueRestrictionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, ExistentialQuantificationTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, ExistentialQuantificationTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AtLeastNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtLeastNumberRestrictionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AtMostNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtMostNumberRestrictionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, ExactNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, ExactNumberRestrictionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, QualifiedAtLeastNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, QualifiedAtLeastNumberRestrictionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, QualifiedAtMostNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, QualifiedAtMostNumberRestrictionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, QualifiedExactNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, QualifiedExactNumberRestrictionTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, RoleValueMapTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, RoleValueMapTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, AgreementTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AgreementTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, RoleFillersTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, RoleFillersTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, OneOfTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, OneOfTag>>(m);
    urd::python::bind_index<Concept<grammar::GrammarFamily, NominalTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, NominalTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, UniversalTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, UniversalTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, IntersectionTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, IntersectionTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, UnionTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, UnionTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, ComplementTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, ComplementTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, InverseTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, InverseTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, CompositionTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, CompositionTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, TransitiveClosureTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, TransitiveClosureTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, ReflexiveTransitiveClosureTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, ReflexiveTransitiveClosureTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, RestrictionTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, RestrictionTag>>(m);
    urd::python::bind_index<Role<grammar::GrammarFamily, IdentityTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, IdentityTag>>(m);
    urd::python::bind_index<Boolean<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_index<Boolean<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_index<Boolean<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_index<Boolean<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_index<Boolean<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_index<Boolean<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_index<Boolean<grammar::GrammarFamily, NonemptyTag>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, NonemptyTag>>(m);
    urd::python::bind_index<Numerical<grammar::GrammarFamily, CountTag>>(m);
    urd::python::bind_data<Numerical<grammar::GrammarFamily, CountTag>>(m);
    urd::python::bind_index<Numerical<grammar::GrammarFamily, DistanceTag>>(m);
    urd::python::bind_data<Numerical<grammar::GrammarFamily, DistanceTag>>(m);

    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, BotTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, BotTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, TopTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, TopTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AtomicStateTag<StaticTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AtomicStateTag<StaticTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AtomicStateTag<FluentTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AtomicStateTag<FluentTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, IntersectionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, IntersectionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, UnionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, UnionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, NegationTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, NegationTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, ValueRestrictionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, ValueRestrictionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, ExistentialQuantificationTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, ExistentialQuantificationTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AtLeastNumberRestrictionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AtLeastNumberRestrictionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AtMostNumberRestrictionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AtMostNumberRestrictionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, ExactNumberRestrictionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, ExactNumberRestrictionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, QualifiedAtLeastNumberRestrictionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, QualifiedAtLeastNumberRestrictionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, QualifiedAtMostNumberRestrictionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, QualifiedAtMostNumberRestrictionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, QualifiedExactNumberRestrictionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, QualifiedExactNumberRestrictionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, RoleValueMapTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, RoleValueMapTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, AgreementTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, AgreementTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, RoleFillersTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, RoleFillersTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, OneOfTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, OneOfTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Concept<grammar::GrammarFamily, NominalTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Concept<grammar::GrammarFamily, NominalTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, UniversalTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, UniversalTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, AtomicStateTag<StaticTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, AtomicStateTag<StaticTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, AtomicStateTag<FluentTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, AtomicStateTag<FluentTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, IntersectionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, IntersectionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, UnionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, UnionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, ComplementTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, ComplementTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, InverseTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, InverseTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, CompositionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, CompositionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, TransitiveClosureTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, TransitiveClosureTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, ReflexiveTransitiveClosureTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, ReflexiveTransitiveClosureTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, RestrictionTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, RestrictionTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Role<grammar::GrammarFamily, IdentityTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Role<grammar::GrammarFamily, IdentityTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Boolean<grammar::GrammarFamily, AtomicStateTag<StaticTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Boolean<grammar::GrammarFamily, AtomicStateTag<StaticTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Boolean<grammar::GrammarFamily, AtomicStateTag<FluentTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Boolean<grammar::GrammarFamily, AtomicStateTag<FluentTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Boolean<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Boolean<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Boolean<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Boolean<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Boolean<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Boolean<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Boolean<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Boolean<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Boolean<grammar::GrammarFamily, NonemptyTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Boolean<grammar::GrammarFamily, NonemptyTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Numerical<grammar::GrammarFamily, CountTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Numerical<grammar::GrammarFamily, CountTag>, grammar::ConstructorRepository>(m);
    urd::python::bind_view<urd::index_view<Numerical<grammar::GrammarFamily, DistanceTag>, grammar::ConstructorRepository>>(m);
    urd::python::bind_builder<Numerical<grammar::GrammarFamily, DistanceTag>, grammar::ConstructorRepository>(m);
}

}  // namespace runir::kr::dl
