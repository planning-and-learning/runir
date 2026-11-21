#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::ps::ext::dl
{

// One translation unit per kind: the three instantiations together need about 6 GB to compile.
void bind_conditions(nb::module_& m, RepositoryBinding& repository)
{
    using runir::kr::DlTag;
    using runir::kr::dl::BooleanTag;
    using runir::kr::dl::NumericalTag;
    using namespace runir::kr::ps::dl;
    urd::python::bind_index<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Positive>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Positive>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Negative>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Negative>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, EqualZero>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, EqualZero>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, GreaterZero>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, GreaterZero>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Positive>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Positive>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Positive>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Negative>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Negative>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, BooleanTag, Negative>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, EqualZero>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, EqualZero>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, EqualZero>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, GreaterZero>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, GreaterZero>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteCondition<ExtFamilyTag, DlTag, NumericalTag, GreaterZero>>(repository);
}

}  // namespace runir::kr::ps::ext::dl
