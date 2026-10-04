#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/rule_variant_data.hpp>
#include <runir/kr/ps/ext/rule_variant_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

void bind_rule_variant(nb::module_& m, RepositoryBinding& repository)
{
    using T = runir::kr::ps::Rule<runir::kr::ExtFamilyTag>;
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, "RuleVariantIndex");
    auto data = nb::class_<Data>(m, "RuleVariantData")
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("symbol", &Data::symbol)
                    .def_rw("variant", &Data::variant);
    ygg::add_comparison(data);
    auto view =
        nb::class_<View>(m, "RuleVariant").def("get_index", &View::get_index).def("get_symbol", &View::get_symbol).def("get_variant", &View::get_variant);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    runir::kr::python::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::ext
