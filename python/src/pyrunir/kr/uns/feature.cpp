#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/uns/feature_index.hpp>
#include <runir/kr/uns/feature_view.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <runir/kr/uns/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::uns
{

void bind_feature(nb::module_& m)
{
    using View = ygg::View<ygg::Index<Feature>, Repository>;
    ygg::bind_index<ygg::Index<Feature>>(m, "FeatureIndex");
    auto cls = nb::class_<View>(m, "Feature")
                   .def("get_index", &View::get_index)
                   .def("get_variant", &View::get_variant)
                   .def("get_feature", &View::get_feature, nb::keep_alive<0, 1>())
                   .def("get_symbol", &View::get_symbol)
                   .def("syntactic_complexity", [](View view) { return syntactic_complexity(view); });
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace runir::kr::uns
