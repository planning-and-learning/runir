#include "../record_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <urd/python/bindings.hpp>

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

    python::bind_record_views<SemanticFamily, ConstructorRepository>(m, BooleanTag {});
    python::bind_record_views<SemanticFamily, ConstructorRepository>(m, NumericalTag {});
}

}  // namespace runir::kr::dl
