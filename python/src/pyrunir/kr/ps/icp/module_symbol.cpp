#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/module_symbol_data.hpp>
#include <runir/kr/ps/icp/module_symbol_view.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::ps::icp
{

void bind_module_symbol(nb::module_& m, RepositoryBinding& repository)
{
    using T = ModuleSymbol;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, "ModuleSymbolIndex");
    auto data = nb::class_<Data>(m, "ModuleSymbolData")
                    .def(nb::init<>())
                    .def(nb::init<::cista::offset::string>(), nb::arg("name"))
                    .def_rw("index", &Data::index)
                    .def_rw("name", &Data::name);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, "ModuleSymbol").def("get_index", &View::index).def("get_name", &View::get_name);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    runir::kr::python::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::icp
