#ifndef RUNIR_KR_DETAIL_RECORD_KEY_HPP_
#define RUNIR_KR_DETAIL_RECORD_KEY_HPP_

#include <concepts>
#include <cstddef>
#include <utility>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::detail
{

template<typename T, typename... Ts>
consteval size_t type_position(ygg::TypeList<Ts...>) noexcept
{
    size_t position = 0;
    static_cast<void>(((std::same_as<T, Ts> ? true : (++position, false)) || ...));
    return position;
}

/// Identifies a record of any type in Types, so one map or set covers the records of all types instead of one
/// container instantiation per type.
using RecordKey = std::pair<size_t, ygg::uint_t>;

template<typename Types, typename T, typename C>
RecordKey record_key(const ygg::View<ygg::Index<T>, C>& view) noexcept
{
    return { type_position<T>(Types {}), view.get_index().get_value() };
}

}  // namespace runir::kr::detail

#endif
