#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/serialization/serialization.hpp>
#include <runir/serialization/types.hpp>
#include <yggdrasil/python/serialization.hpp>

using namespace nb::literals;

namespace runir::serialization
{
using ygg::serialization::Dictionaries;

void bind_module_definitions(nb::module_& m)
{
    m.def("register_table",
          [](Dictionaries& dictionaries, nb::type_object native_type, const std::string& name, const std::string& prefix)
          { ygg::python::register_table(dictionaries, native_type, name, prefix, RegisteredTypes {}); },
          "dictionaries"_a,
          "native_type"_a,
          "name"_a,
          "prefix"_a);
    m.def("serialize",
          [](Dictionaries& dictionaries, nb::handle value) { return ygg::python::serialize(dictionaries, value, SerializedTypes {}); },
          "dictionaries"_a,
          "value"_a,
          nb::keep_alive<1, 2>());
    m.def("table",
          [](Dictionaries& dictionaries, nb::type_object native_type) { return ygg::python::table(dictionaries, native_type, RegisteredTypes {}); },
          "dictionaries"_a,
          "native_type"_a);
}

}  // namespace runir::serialization
