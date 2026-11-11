#include "bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/uns/classifier_view.hpp>
#include <runir/kr/uns/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::uns
{

void bind_classifier_literal(nb::module_& m)
{
    using View = urd::index_view<ClassifierLiteral, Repository>;
    urd::python::bind_index<urd::index<ClassifierLiteral>>(m, "ClassifierLiteralIndex");
    auto cls =
        nb::class_<View>(m, "ClassifierLiteral").def("index", &View::index).def("get_feature", &View::get_feature).def("get_polarity", &View::get_polarity);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
}

}  // namespace runir::kr::uns
