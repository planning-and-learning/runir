#ifndef RUNIR_KR_UNS_FEATURE_DATA_HPP_
#define RUNIR_KR_UNS_FEATURE_DATA_HPP_

#include "runir/kr/uns/dl/feature_index.hpp"
#include "runir/kr/uns/feature_index.hpp"

#include <cista/containers/variant.h>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

// The variant lists the available feature languages; extend it to support more languages.
template<>
struct Data<runir::kr::uns::Feature>
{
    using Variant = ::cista::offset::variant<Index<runir::kr::uns::dl::Feature>>;

    Index<runir::kr::uns::Feature> index;
    Variant value;

    Data() = default;
    Data(Variant value_) : index(), value(std::move(value_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(value);
    }

    auto cista_members() const noexcept { return std::tie(index, value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

}  // namespace ygg

#endif
