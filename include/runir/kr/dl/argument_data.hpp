#ifndef RUNIR_KR_DL_ARGUMENT_DATA_HPP_
#define RUNIR_KR_DL_ARGUMENT_DATA_HPP_

#include "runir/kr/dl/argument_index.hpp"

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
    Data(const std::string& name_, runir::kr::dl::ArgumentIdentifier<Category> identifier_) : index(), name(name_), identifier(identifier_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(name);
        ygg::clear(identifier);
    }

    auto cista_members() const noexcept { return std::tie(index, name, identifier); }
    auto identifying_members() const noexcept { return std::tie(name, identifier); }
};

}  // namespace ygg

#endif
