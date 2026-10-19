#ifndef RUNIR_SERIALIZATION_TYPES_VIEWS_HPP_
#define RUNIR_SERIALIZATION_TYPES_VIEWS_HPP_

#include <type_traits>
#include <yggdrasil/core/concepts.hpp>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::serialization
{

template<typename Context, typename T>
using IndexView = ygg::View<ygg::Index<T>, Context>;

template<typename T>
using HashableTypeList = std::conditional_t<ygg::Hashable<T>, ygg::TypeList<T>, ygg::TypeList<>>;

}  // namespace runir::serialization

#endif
