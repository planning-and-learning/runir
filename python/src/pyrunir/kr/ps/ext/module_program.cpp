#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/module_program_data.hpp>
#include <runir/kr/ps/ext/module_program_view.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

void bind_module_program(nb::module_& m, RepositoryBinding& repository)
{
    using T = ModuleProgram;
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, "ModuleProgramIndex");
    auto data = nb::class_<Data>(m, "ModuleProgramData").def(nb::init<>()).def_rw("index", &Data::index).def_rw("entry_module", &Data::entry_module);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, "ModuleProgram")
                    .def("get_index", &View::get_index)
                    .def("get_entry_module", &View::get_entry_module, nb::keep_alive<0, 1>())
                    .def("get_modules", &View::get_modules)
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::ext::syntactic_complexity(value); });
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<T, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::ext
