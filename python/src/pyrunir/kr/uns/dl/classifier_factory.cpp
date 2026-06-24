#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/uns/dl/classifier_factory.hpp>

namespace runir::kr::uns::dl
{

void bind_classifier_factory(nb::module_& m)
{
    nb::class_<ClassifierFactory>(m, "ClassifierFactory")
        .def_static("create_empty", &ClassifierFactory::create_empty, nb::arg("repository"), nb::keep_alive<0, 1>())
        .def_static("create_empty_description", &ClassifierFactory::create_empty_description);
}

}  // namespace runir::kr::uns::dl
