#ifndef PYRUNIR_KR_BINDING_UTILS_HPP_
#define PYRUNIR_KR_BINDING_UTILS_HPP_

#include <cstddef>
#include <nanobind/nanobind.h>
#include <utility>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <urd/python/owner.hpp>

namespace runir::kr::python
{

template<typename T, typename Repository>
void bind_insert(nanobind::class_<Repository>& repository)
{
    namespace nb = nanobind;
    repository.def("insert", [](Repository& self, urd::data<T>& data) { return ygg::formalism::insert(self, data); }, nb::arg("data"));
}

namespace detail
{
template<typename Class, typename Factory, typename... Args, size_t... Is>
void def_new_keeping_arguments_alive(Class& cls, Factory factory, std::index_sequence<Is...> /*positions*/, const Args&... args)
{
    // nanobind keep_alive positions: 0 is the result, 1 is self, and the constructor arguments start at 2.
    cls.def(nanobind::new_(std::move(factory)), args..., nanobind::keep_alive<0, Is + 2>()...);
}
}

/// Binds a constructor whose result keeps every one of its arguments alive; pass one nb::arg per factory parameter.
template<typename Class, typename Factory, typename... Args>
void def_new_keeping_arguments_alive(Class& cls, Factory factory, const Args&... args)
{ detail::def_new_keeping_arguments_alive(cls, std::move(factory), std::index_sequence_for<Args...> {}, args...); }

}

#endif
