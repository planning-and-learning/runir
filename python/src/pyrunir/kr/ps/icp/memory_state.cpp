#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/memory_state_data.hpp>
#include <runir/kr/ps/icp/memory_state_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::ps::icp
{

void bind_memory_state(nb::module_& m, RepositoryBinding& repository)
{
    using T = MemoryState;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, "MemoryStateIndex");
    auto data = nb::class_<Data>(m, "MemoryStateData")
                    .def(nb::init<>())
                    .def(nb::init<urd::flat_string>(), nb::arg("name"))
                    .def_rw("index", &Data::index)
                    .def_rw("name", &Data::name);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, "MemoryState").def("get_index", &View::index).def("get_name", &View::get_name);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    ygg::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::icp
