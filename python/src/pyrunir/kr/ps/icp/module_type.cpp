#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/module_data.hpp>
#include <runir/kr/ps/icp/module_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/syntactic_complexity.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::icp
{

void bind_module(nb::module_& m, RepositoryBinding& repository)
{
    urd::python::bind_data<Module>(m);
    urd::python::bind_view<ModuleView>(m);
    m.def("syntactic_complexity", [](ModuleView module_) { return runir::kr::ps::icp::syntactic_complexity(module_); }, nb::arg("module"));
    ygg::bind_insert<Module>(repository);
}

}  // namespace runir::kr::ps::icp
