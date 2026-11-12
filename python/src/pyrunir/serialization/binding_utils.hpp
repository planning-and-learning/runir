#ifndef PYRUNIR_SERIALIZATION_BINDING_UTILS_HPP_
#define PYRUNIR_SERIALIZATION_BINDING_UTILS_HPP_

#include "module.hpp"

#include <runir/kr/dl/declarations.hpp>
#include <runir/serialization/types/views.hpp>
#include <yggdrasil/python/serialization.hpp>

namespace runir::serialization
{

template<typename Types>
using DlComponentViews = boost::mp11::mp_transform_q<boost::mp11::mp_bind_front<IndexView, kr::dl::ConstructorRepository>, Types>;

template<typename... Ts>
void bind_serialized_types(nb::module_& m, boost::mp11::mp_list<Ts...> types)
{ ygg::python::bind_serialization(m, boost::mp11::mp_list<> {}, types, types); }

template<typename... Ts>
void bind_entity_types(nb::module_& m, boost::mp11::mp_list<Ts...> types)
{
    using Registered = boost::mp11::mp_append<HashableTypeList<Ts>...>;
    ygg::python::bind_serialization(m, Registered {}, types, types);
}

}  // namespace runir::serialization

#endif
