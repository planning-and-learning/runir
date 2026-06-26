#ifndef RUNIR_KR_UNS_DL_FEATURE_DATA_HPP_
#define RUNIR_KR_UNS_DL_FEATURE_DATA_HPP_

#include "runir/kr/dl/constructor_index.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/uns/dl/feature_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::uns::dl::Feature>
{
    Index<runir::kr::uns::dl::Feature> index;
    Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>> feature;
    ::cista::offset::string symbol;

    Data() = default;
    Data(Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>> feature_,
         ::cista::offset::string symbol_) :
        index(),
        feature(feature_),
        symbol(std::move(symbol_))
    {
    }
    Data(Index<runir::kr::dl::Constructor<runir::kr::UnsFamilyTag, runir::kr::dl::BooleanTag>> feature_,
         const std::string& symbol_) :
        index(),
        feature(feature_),
        symbol(symbol_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(feature);
        ygg::clear(symbol);
    }

    auto cista_members() const noexcept { return std::tie(index, feature, symbol); }
    auto identifying_members() const noexcept { return std::tie(feature, symbol); }
};

}  // namespace ygg

#endif
