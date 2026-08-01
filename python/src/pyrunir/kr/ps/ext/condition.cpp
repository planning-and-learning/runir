#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/condition_view.hpp>
#include <runir/kr/ps/ext/condition_data.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

void bind_condition(nb::module_& m, RepositoryBinding& repository)
{
    using T = runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>;
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, "ConditionVariantIndex");
    auto data = nb::class_<Data>(m, "ConditionVariantData").def(nb::init<>()).def_rw("index", &Data::index).def_rw("value", &Data::value);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, "ConditionVariant").def("get_index", &View::get_index).def("get_variant", &View::get_variant);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<T, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::ext
