#include "bindings.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>
#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/dl/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/ps/ext/detail/repository_instantiations.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/program_data.hpp>
#include <runir/kr/ps/ext/program_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

void bind_program(nb::module_& m, RepositoryBinding& repository)
{
    using T = Program;
    using Data = urd::data<T>;
    using View = urd::view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, "ProgramIndex");
    auto data = nb::class_<Data>(m, "ProgramData")
                    .def(nb::init<>())
                    .def(nb::init<urd::index<Module>, urd::flat_vector<urd::index<Module>>>(), nb::arg("entry_module"), nb::arg("modules"))
                    .def(urd::python::init_from_views<urd::view<Module, Repository>, const urd::Store<urd::sequence<urd::view<Module, Repository>>>&>(),
                         nb::arg("entry_module"),
                         nb::arg("modules"))
                    .def_rw("index", &Data::index)
                    .def_rw("entry_module", &Data::entry_module)
                    .def_rw("modules", &Data::modules);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, "Program")
                    .def("get_index", &View::index)
                    .def("get_entry_module", &View::get_entry_module, nb::keep_alive<0, 1>())
                    .def("get_modules", &View::get_modules)
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::ext::syntactic_complexity(value); });
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    runir::kr::python::bind_ps_optimize<runir::kr::ExtFamilyTag, Repository>(view);
    runir::kr::python::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::ext
