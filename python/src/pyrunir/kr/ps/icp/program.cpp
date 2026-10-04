#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/program_data.hpp>
#include <runir/kr/ps/icp/program_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::icp
{

using namespace nanobind::literals;

void bind_program(nb::module_& m, RepositoryBinding& repository)
{
    using T = Program;
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, "ProgramIndex");
    auto data = nb::class_<Data>(m, "ProgramData").def(nb::init<>()).def_rw("index", &Data::index).def_rw("module", &Data::module);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, "Program")
                    .def("get_index", &View::get_index)
                    .def("get_entry_module", &View::get_entry_module, nb::keep_alive<0, 1>())
                    .def("get_module", &View::get_module, nb::keep_alive<0, 1>())
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::icp::syntactic_complexity(value); });
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    runir::kr::python::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::icp
