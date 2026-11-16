#include "bindings.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_data.hpp>
#include <runir/kr/ps/ext/module_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::ext
{

void bind_module(nb::module_& m, RepositoryBinding& repository)
{
    urd::python::bind_data<Module>(m);
    urd::python::bind_view<ModuleView>(m);
    m.def("syntactic_complexity", [](ModuleView module_) { return runir::kr::ps::ext::syntactic_complexity(module_); }, nb::arg("module"));
    runir::kr::python::bind_ps_optimize<runir::kr::ExtFamilyTag, Repository, ModuleView>(m);
    ygg::bind_insert<Module>(repository);
}

}  // namespace runir::kr::ps::ext
