#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::ps::ext::dl
{

// One translation unit per kind: the three instantiations together need about 6 GB to compile.
void bind_effects(nb::module_& m, RepositoryBinding& repository)
{
    using runir::kr::dl::BooleanTag;
    using runir::kr::dl::NumericalTag;
    using namespace runir::kr::ps::dl;
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Positive>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Positive>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Negative>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Negative>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Unchanged>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Increases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Increases>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Decreases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Decreases>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Unchanged>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Positive>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Positive>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Positive>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Negative>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Negative>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Negative>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Unchanged>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Unchanged>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<ExtFamilyTag, BooleanTag, Unchanged>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Increases>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Increases>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Increases>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Decreases>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Decreases>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Decreases>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Unchanged>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Unchanged>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<ExtFamilyTag, NumericalTag, Unchanged>>(repository);
}

}  // namespace runir::kr::ps::ext::dl
