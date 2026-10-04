#ifndef PYRUNIR_KR_BINDING_UTILS_HPP_
#define PYRUNIR_KR_BINDING_UTILS_HPP_

#include <nanobind/nanobind.h>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::python
{

inline auto make_owner_retainer()
{
    namespace nb = nanobind;
    return nb::cpp_function([](nb::object value, nb::handle) { return value; }, nb::keep_alive<0, 2>());
}

template<typename T, typename Repository>
auto get_or_create_data(Repository& repository, ygg::Data<T>& data)
{
    return ygg::formalism::get_or_create(repository, data).first;
}

}

#endif
