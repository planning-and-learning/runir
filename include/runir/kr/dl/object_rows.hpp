#ifndef RUNIR_KR_DL_OBJECT_ROWS_HPP_
#define RUNIR_KR_DL_OBJECT_ROWS_HPP_

#include <cstddef>
#include <ranges>
#include <span>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/database/syntax/row.hpp>

namespace runir::kr::dl
{
/// Encodes a range of object indices as a row of uint_t columns, the type yggdrasil stores indices as.
template<std::ranges::sized_range R>
std::span<const std::byte> encode_object_row(const R& objects, std::vector<std::byte>& buffer)
{
    return ygg::database::encode_row<ygg::uint_t>(std::views::transform(objects, [](auto object) { return object.get_value(); }), buffer);
}
}  // namespace runir::kr::dl

#endif
