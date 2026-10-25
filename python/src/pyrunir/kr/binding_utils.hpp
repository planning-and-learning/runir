#ifndef PYRUNIR_KR_BINDING_UTILS_HPP_
#define PYRUNIR_KR_BINDING_UTILS_HPP_

#include <cstddef>
#include <nanobind/nanobind.h>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/python/owner.hpp>

namespace runir::kr::python
{

template<typename T, typename Repository>
void bind_insert(nanobind::class_<Repository>& repository)
{
    namespace nb = nanobind;
    using View = ygg::View<ygg::Index<T>, Repository>;
    const auto retainer = ygg::python::make_owner_retainer();
    repository.def(
        "insert",
        [retainer](nb::typed<nb::handle, Repository> owner, ygg::Data<T>& data)
        {
            const auto [view, inserted] = ygg::formalism::insert(nb::cast<Repository&>(owner), data);
            auto result = nb::make_tuple(nb::cast(view), inserted);
            return nb::typed<nb::tuple, View, bool>(ygg::python::retain_owner_tree(result, owner, retainer));
        },
        nb::arg("data"));
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
