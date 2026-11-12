#ifndef RUNIR_KR_DL_OBJECT_ROWS_HPP_
#define RUNIR_KR_DL_OBJECT_ROWS_HPP_

#include <cstddef>
#include <ranges>
#include <span>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/formalism/values.hpp>

namespace runir::kr::dl
{
/// Encodes a range of object indices as a row of object columns.
template<std::ranges::sized_range R>
std::span<const ygg::Value> encode_object_row(const R& objects, urd::Store<urd::sequence<ygg::Value>>& buffer)
{
    buffer.clear();
    for (const auto object : objects)
        buffer.push_back(ygg::encode_value(object));
    return buffer;
}
}  // namespace runir::kr::dl

#endif
