#ifndef RUNIR_SERIALIZATION_TYPES_VIEWS_HPP_
#define RUNIR_SERIALIZATION_TYPES_VIEWS_HPP_

#include <boost/mp11.hpp>
#include <type_traits>
#include <urd/elements/flat.hpp>

namespace runir::serialization
{

template<typename Context, typename T>
using IndexView = urd::index_view<T, Context>;

template<typename T>
using HashableTypeList = std::conditional_t<urd::Hashable<T>, boost::mp11::mp_list<T>, boost::mp11::mp_list<>>;

}  // namespace runir::serialization

#endif
