#ifndef PYRUNIR_KR_BINDING_UTILS_HPP_
#define PYRUNIR_KR_BINDING_UTILS_HPP_

#include <cstddef>
#include <nanobind/nanobind.h>
#include <urd/elements/concepts.hpp>
#include <urd/elements/flat.hpp>
#include <urd/elements/view.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::python
{

namespace detail
{
template<typename Class, typename Factory, typename... Args, size_t... Is>
void def_new_keeping_arguments_alive(Class& cls, Factory factory, std::index_sequence<Is...> /*positions*/, const Args&... args)
{
    // nanobind keep_alive positions: 0 is the result, 1 is self, and the constructor arguments start at 2.
    cls.def(nanobind::new_(std::move(factory)), args..., nanobind::keep_alive<0, Is + 2>()...);
}
}

/// What a Python __init__ takes for a record member of type M: its view (urd::resolve: through the repository, which
/// resolves what other repositories hold through them), a list of them as a list of their views.
template<typename M, typename Repository>
struct FromView
{
    using type = urd::resolved_t<M, Repository>;
};
template<typename M, typename Repository>
    requires urd::resolving<M>
struct FromView<urd::flat_vector<M>, Repository>
{
    using type = const urd::Store<urd::sequence<urd::resolved_t<M, Repository>>>&;
};

/// The record's fields (urd::Fields), under their names, as read/write attributes and as the arguments of its __init__
/// from values and, where a field resolves, from views (Repository resolves them).
template<typename Repository, typename Data>
void def_fields(nanobind::class_<Data>& cls)
{
    urd::python::bind_fields(cls);
    [&]<typename... Fs>(urd::Fields<Fs...> /*fields*/)
    {
        if constexpr ((urd::resolving<urd::member_type_t<Fs::member>> || ...))
            cls.def(urd::python::init_from_views<typename FromView<urd::member_type_t<Fs::member>, Repository>::type...>(), nanobind::arg(Fs::c_name)...);
    }(Data::fields());
}

/// Binds a constructor whose result keeps every one of its arguments alive; pass one nb::arg per factory parameter.
template<typename Class, typename Factory, typename... Args>
void def_new_keeping_arguments_alive(Class& cls, Factory factory, const Args&... args)
{ detail::def_new_keeping_arguments_alive(cls, std::move(factory), std::index_sequence_for<Args...> {}, args...); }

}

#endif
