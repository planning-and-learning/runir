#ifndef PYRUNIR_KR_BINDING_UTILS_HPP_
#define PYRUNIR_KR_BINDING_UTILS_HPP_

#include <nanobind/nanobind.h>
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

}

#endif
