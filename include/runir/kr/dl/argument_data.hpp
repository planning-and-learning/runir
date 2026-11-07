#ifndef RUNIR_KR_DL_ARGUMENT_DATA_HPP_
#define RUNIR_KR_DL_ARGUMENT_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::dl::Argument<Category>>
{
    urd::index<runir::kr::dl::Argument<Category>> index;
    urd::flat_string name;
    runir::kr::dl::ArgumentIdentifier<Category> identifier;

    data() = default;
    data(urd::flat_string name_, runir::kr::dl::ArgumentIdentifier<Category> identifier_) : index(), name(std::move(name_)), identifier(identifier_) {}

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name, &data::identifier> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::name, &data::identifier> {}; }
};

}  // namespace urd

#endif
