#ifndef RUNIR_KR_DL_CONSTRUCTOR_BUILDER_HPP_
#define RUNIR_KR_DL_CONSTRUCTOR_BUILDER_HPP_

#include "runir/kr/dl/constructor_data.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Constructor: its record's members, each in its built form (urd::built_t).
template<runir::kr::dl::CategoryTag Category, typename C>
struct builder<runir::kr::dl::Constructor<Category>, C>
{
    urd::index<runir::kr::dl::Constructor<Category>> index;
    urd::built_t<runir::kr::dl::detail::ConstructorDataVariant<Category>, C> variant;
    urd::built_t<bool, C> is_static = false;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"variant", &builder::variant>, urd::field<"is_static", &builder::is_static>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::variant> {}; }
};

}  // namespace urd

#endif
