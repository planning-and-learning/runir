#include "../module.hpp"
#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/formatter.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_cnf_grammar_records(nb::module_& m)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, BotTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, TopTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, IntersectionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, UnionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, NegationTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, ValueRestrictionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, ExistentialQuantificationTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AtLeastNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AtMostNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, ExactNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, QualifiedAtLeastNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, QualifiedAtMostNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, QualifiedExactNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, RoleValueMapTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, AgreementTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, RoleFillersTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, OneOfTag>>(m);
    urd::python::bind_data<Concept<cnf_grammar::CnfGrammarFamily, NominalTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, UniversalTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, IntersectionTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, UnionTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, ComplementTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, InverseTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, CompositionTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, TransitiveClosureTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, ReflexiveTransitiveClosureTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, RestrictionTag>>(m);
    urd::python::bind_data<Role<cnf_grammar::CnfGrammarFamily, IdentityTag>>(m);
    urd::python::bind_data<Boolean<cnf_grammar::CnfGrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Boolean<cnf_grammar::CnfGrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Boolean<cnf_grammar::CnfGrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Boolean<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Boolean<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Boolean<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Boolean<cnf_grammar::CnfGrammarFamily, NonemptyTag>>(m);
    urd::python::bind_data<Numerical<cnf_grammar::CnfGrammarFamily, CountTag>>(m);
    urd::python::bind_data<Numerical<cnf_grammar::CnfGrammarFamily, DistanceTag>>(m);

    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, BotTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, TopTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AtomicStateTag<StaticTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AtomicStateTag<FluentTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AtomicStateTag<DerivedTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<StaticTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<FluentTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<DerivedTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, IntersectionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, UnionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, NegationTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, ValueRestrictionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, ExistentialQuantificationTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AtLeastNumberRestrictionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AtMostNumberRestrictionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, ExactNumberRestrictionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, QualifiedAtLeastNumberRestrictionTag>, cnf_grammar::ConstructorRepository>>(
        m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, QualifiedAtMostNumberRestrictionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, QualifiedExactNumberRestrictionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, RoleValueMapTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, AgreementTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, RoleFillersTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, OneOfTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Concept<cnf_grammar::CnfGrammarFamily, NominalTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, UniversalTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, AtomicStateTag<StaticTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, AtomicStateTag<FluentTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, AtomicStateTag<DerivedTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<StaticTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<FluentTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<DerivedTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, IntersectionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, UnionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, ComplementTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, InverseTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, CompositionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, TransitiveClosureTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, ReflexiveTransitiveClosureTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, RestrictionTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Role<cnf_grammar::CnfGrammarFamily, IdentityTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<cnf_grammar::CnfGrammarFamily, AtomicStateTag<StaticTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<cnf_grammar::CnfGrammarFamily, AtomicStateTag<FluentTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<cnf_grammar::CnfGrammarFamily, AtomicStateTag<DerivedTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<StaticTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<FluentTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<cnf_grammar::CnfGrammarFamily, AtomicGoalTag<DerivedTag>>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<cnf_grammar::CnfGrammarFamily, NonemptyTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<cnf_grammar::CnfGrammarFamily, CountTag>, cnf_grammar::ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<cnf_grammar::CnfGrammarFamily, DistanceTag>, cnf_grammar::ConstructorRepository>>(m);
}

}  // namespace runir::kr::dl
