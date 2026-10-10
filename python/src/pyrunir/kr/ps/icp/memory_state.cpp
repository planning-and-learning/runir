#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/memory_state_data.hpp>
#include <runir/kr/ps/icp/memory_state_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::icp
{

using namespace nanobind::literals;

void bind_memory_state(nb::module_& m, RepositoryBinding& repository)
{
    using T = MemoryState;
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, "MemoryStateIndex");
    auto data = nb::class_<Data>(m, "MemoryStateData")
                    .def(nb::init<>())
                    .def(nb::init<::cista::offset::string>(), nb::arg("name"))
                    .def_rw("index", &Data::index)
                    .def_rw("name", &Data::name);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, "MemoryState").def("get_index", &View::get_index).def("get_name", &View::get_name);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    runir::kr::python::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::icp
