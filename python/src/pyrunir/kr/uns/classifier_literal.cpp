#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/uns/classifier_index.hpp>
#include <runir/kr/uns/classifier_view.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::uns
{

void bind_classifier_literal(nb::module_& m)
{
    using View = ygg::View<ygg::Index<ClassifierLiteral>, Repository>;
    ygg::bind_index<ygg::Index<ClassifierLiteral>>(m, "ClassifierLiteralIndex");
    auto cls = nb::class_<View>(m, "ClassifierLiteral")
                   .def("get_index", &View::get_index)
                   .def("get_feature", &View::get_feature, nb::keep_alive<0, 1>())
                   .def("get_polarity", &View::get_polarity);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace runir::kr::uns
