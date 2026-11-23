#include "bindings.hpp"
#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"
#include "pyrunir/kr/ps/ext/conversions.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_data.hpp>
#include <runir/kr/ps/ext/module_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
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
    urd::python::bind_index<Module>(m);
    urd::python::bind_data<Module>(m);
    urd::python::bind_view<ModuleView>(m);
    urd::python::bind_builder<Module, Repository>(m);
    runir::kr::python::bind_ps_optimize<runir::kr::ExtFamilyTag, Repository, ModuleView>(m);
    ygg::bind_insert<Module>(repository);
}

}  // namespace runir::kr::ps::ext
