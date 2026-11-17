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

void bind_semantics_feature_records(nb::module_& m)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    urd::python::bind_data<Boolean<SemanticFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, NonemptyTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, ArgumentTag<BooleanTag>>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, BooleanEqTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, BooleanNeTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, BooleanLtTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, BooleanLeTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, BooleanGtTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, BooleanGeTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, NumericalEqTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, NumericalNeTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, NumericalLtTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, NumericalLeTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, NumericalGtTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, NumericalGeTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, BooleanConstantTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, AndTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, OrTag>>(m);
    urd::python::bind_data<Boolean<SemanticFamily, NotTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, CountTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, DistanceTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, ArgumentTag<NumericalTag>>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, NumericalConstantTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, AddTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, SubTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, MulTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, DivTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, MinTag>>(m);
    urd::python::bind_data<Numerical<SemanticFamily, MaxTag>>(m);

    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, AtomicStateTag<StaticTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, AtomicStateTag<FluentTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, AtomicStateTag<DerivedTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, AtomicGoalTag<StaticTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, AtomicGoalTag<FluentTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, AtomicGoalTag<DerivedTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, NonemptyTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, ArgumentTag<BooleanTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, BooleanEqTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, BooleanNeTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, BooleanLtTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, BooleanLeTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, BooleanGtTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, BooleanGeTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, NumericalEqTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, NumericalNeTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, NumericalLtTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, NumericalLeTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, NumericalGtTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, NumericalGeTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, BooleanConstantTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, AndTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, OrTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Boolean<SemanticFamily, NotTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, CountTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, DistanceTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, ArgumentTag<NumericalTag>>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, NumericalConstantTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, AddTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, SubTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, MulTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, DivTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, MinTag>, ConstructorRepository>>(m);
    urd::python::bind_view<urd::index_view<Numerical<SemanticFamily, MaxTag>, ConstructorRepository>>(m);
}

}  // namespace runir::kr::dl
