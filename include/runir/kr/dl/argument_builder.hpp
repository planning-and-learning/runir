#ifndef RUNIR_KR_DL_ARGUMENT_BUILDER_HPP_
#define RUNIR_KR_DL_ARGUMENT_BUILDER_HPP_

#include "runir/kr/dl/argument_data.hpp"
#include "runir/kr/dl/argument_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Argument: its record's members, each in its built form (urd::built_t).
template<runir::kr::dl::CategoryTag Category, typename C>
struct builder<runir::kr::dl::Argument<Category>, C>
{
    urd::index<runir::kr::dl::Argument<Category>> index;
    urd::built_t<urd::flat_string, C> name;
    urd::built_t<runir::kr::dl::ArgumentIdentifier<Category>, C> identifier;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"name", &builder::name>, urd::field<"identifier", &builder::identifier>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
