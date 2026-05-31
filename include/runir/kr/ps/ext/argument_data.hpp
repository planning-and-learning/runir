#ifndef RUNIR_KR_PS_EXT_ARGUMENT_DATA_HPP_
#define RUNIR_KR_PS_EXT_ARGUMENT_DATA_HPP_

#include "runir/kr/dl/ext/declarations.hpp"
#include "runir/kr/ps/ext/argument_index.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <utility>

namespace ygg
{

template<runir::kr::dl::CategoryTag Category>
struct Data<runir::kr::ps::ext::Argument<Category>>
{
    Index<runir::kr::ps::ext::Argument<Category>> index;
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
