#ifndef PYRUNIR_SERIALIZATION_BINDING_UTILS_HPP_
#define PYRUNIR_SERIALIZATION_BINDING_UTILS_HPP_

#include "module.hpp"

#include <runir/serialization/serialization.hpp>
#include <runir/serialization/types.hpp>
#include <yggdrasil/python/serialization.hpp>

namespace runir::serialization
{

template<kr::FamilyTag Family, typename Types>
using DlComponentViews = ygg::MapTypeListSecondT<IndexView, kr::dl::ConstructorRepositoryFor<Family>, Types>;

template<typename... Ts>
void bind_serialized_types(nb::module_& m, ygg::TypeList<Ts...> types)
{
    ygg::python::bind_serialization(m, ygg::TypeList<> {}, types, types);
}

template<typename... Ts>
void bind_entity_types(nb::module_& m, ygg::TypeList<Ts...> types)
{
    using Registered = ygg::ConcatTypeListsT<HashableTypeList<Ts>...>;
    ygg::python::bind_serialization(m, Registered {}, types, types);
}

}  // namespace runir::serialization

#endif
