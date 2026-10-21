#ifndef RUNIR_KR_DL_ARGUMENT_DATA_HPP_
#define RUNIR_KR_DL_ARGUMENT_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::dl::Argument<Category>>
{
    Index<runir::kr::dl::Argument<Category>> index;
    ::cista::offset::string name;
    runir::kr::dl::ArgumentIdentifier<Category> identifier;

    Data() = default;
    Data(::cista::offset::string name_, runir::kr::dl::ArgumentIdentifier<Category> identifier_) : index(), name(std::move(name_)), identifier(identifier_) {}

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::name, &Data::identifier> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::name, &Data::identifier> {}; }
    void clear() noexcept
    {
        ygg::clear_members(*this);
    }
};

}  // namespace ygg

#endif
