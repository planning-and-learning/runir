#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"

#include <runir/kr/dl/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/ps/icp/detail/repository_instantiations.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/program_data.hpp>
#include <runir/kr/ps/icp/program_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::ps::icp
{

void bind_program(nb::module_& m, RepositoryBinding& repository)
{
    using T = Program;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, "ProgramIndex");
    auto data = nb::class_<Data>(m, "ProgramData")
                    .def(nb::init<>())
                    .def(nb::init<urd::index<Module>>(), nb::arg("module"))
                    .def(urd::python::init_from_views<urd::index_view<Module, Repository>>(), nb::arg("module"))
                    .def_rw("index", &Data::index)
                    .def_rw("module", &Data::module);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, "Program")
                    .def("get_index", &View::index)
                    .def("get_entry_module", &View::get_entry_module, nb::keep_alive<0, 1>())
                    .def("get_module", &View::get_module, nb::keep_alive<0, 1>())
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::icp::syntactic_complexity(value); });
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    runir::kr::python::bind_ps_optimize<runir::kr::IcpFamilyTag, Repository>(view);
    runir::kr::python::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::icp
