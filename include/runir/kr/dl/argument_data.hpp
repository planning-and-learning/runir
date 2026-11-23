#ifndef RUNIR_KR_DL_ARGUMENT_DATA_HPP_
#define RUNIR_KR_DL_ARGUMENT_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cista/containers/string.h>
#include <string>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<runir::kr::dl::CategoryTag Category>
struct data<runir::kr::dl::Argument<Category>>
{
    urd::key<runir::kr::dl::Argument<Category>> index;
    urd::flat_string name;
    runir::kr::dl::ArgumentIdentifier<Category> identifier;

    data() = default;

    data(urd::flat_string name_, runir::kr::dl::ArgumentIdentifier<Category> identifier_) : index(), name(std::move(name_)), identifier(identifier_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &data::name>, urd::field<"identifier", &data::identifier>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::name, &data::identifier> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Argument<Category>> key() const noexcept { return urd::key<runir::kr::dl::Argument<Category>>(index); }
};

}  // namespace urd

#endif
