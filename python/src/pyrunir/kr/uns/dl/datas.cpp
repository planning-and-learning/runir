#include "pyrunir/kr/uns/dl/module.hpp"

#include <runir/kr/uns/dl/feature_data.hpp>
#include <string>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::uns::dl
{
namespace
{

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = ygg::Data<T>;

    auto cls = nb::class_<Data>(m, name.c_str()).def(nb::init<>());
    ygg::add_comparison(cls);

    if constexpr (requires { &Data::index; })
        cls.def_rw("index", &Data::index);
    if constexpr (requires { &Data::feature; })
        cls.def_rw("feature", &Data::feature);
    if constexpr (requires { &Data::symbol; })
        cls.def_rw("symbol", &Data::symbol);
}

}  // namespace

void bind_datas(nb::module_& m) { bind_data<runir::kr::uns::dl::Feature>(m, "FeatureData"); }

}  // namespace runir::kr::uns::dl
