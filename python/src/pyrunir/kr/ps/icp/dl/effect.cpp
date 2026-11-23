#include "bindings.hpp"
#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"
#include "pyrunir/kr/ps/icp/conversions.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::ps::icp::dl
{

// One translation unit per kind: the three instantiations together need about 6 GB to compile.
void bind_effects(nb::module_& m, RepositoryBinding& repository)
{
    using runir::kr::dl::BooleanTag;
    using runir::kr::dl::NumericalTag;
    using namespace runir::kr::ps::dl;
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Positive>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Positive>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Negative>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Negative>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Unchanged>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Increases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Increases>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Decreases>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Decreases>>(m);
    urd::python::bind_index<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Unchanged>>(m);
    urd::python::bind_data<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Unchanged>>(m);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Positive>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Positive>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Positive>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Negative>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Negative>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Negative>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Unchanged>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Unchanged>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<IcpFamilyTag, BooleanTag, Unchanged>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Increases>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Increases>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Increases>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Decreases>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Decreases>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Decreases>>(repository);
    urd::python::bind_view<urd::index_view<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Unchanged>, Repository>>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Unchanged>, Repository>(m);
    ygg::bind_insert<runir::kr::ps::ConcreteEffect<IcpFamilyTag, NumericalTag, Unchanged>>(repository);
}

}  // namespace runir::kr::ps::icp::dl
