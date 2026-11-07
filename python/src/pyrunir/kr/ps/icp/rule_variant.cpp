#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <nanobind/stl/variant.h>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/rule_variant_data.hpp>
#include <runir/kr/ps/icp/rule_variant_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::ps::icp
{

void bind_rule_variant(nb::module_& m, RepositoryBinding& repository)
{
    using T = runir::kr::ps::Rule<runir::kr::IcpFamilyTag>;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, "RuleVariantIndex");
    auto data = nb::class_<Data>(m, "RuleVariantData")
                    .def(nb::init<>())
                    .def(nb::init<urd::flat_string, Data::Variant>(), nb::arg("symbol"), nb::arg("variant"))
                    .def(urd::python::init_from_views<urd::flat_string, urd::resolved_t<Data::Variant, Repository>>(), nb::arg("symbol"), nb::arg("variant"))
                    .def_rw("index", &Data::index)
                    .def_rw("symbol", &Data::symbol)
                    .def_rw("variant", &Data::variant);
    urd::python::add_comparison(data);
    auto view =
        nb::class_<View>(m, "RuleVariant").def("get_index", &View::index).def("get_symbol", &View::get_symbol).def("get_variant", &View::get_variant);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    ygg::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::icp
