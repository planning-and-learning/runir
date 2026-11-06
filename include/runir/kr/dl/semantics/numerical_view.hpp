#ifndef RUNIR_SEMANTICS_NUMERICAL_VIEW_HPP_
#define RUNIR_SEMANTICS_NUMERICAL_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/numerical_data.hpp"

#include <concepts>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename Tag, typename V>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
struct view_mixin<runir::kr::dl::Numerical<Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_argument() const noexcept
        requires std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::NumericalTag>>
    { return self().template get<&urd::record_of<V>::reference>(); }

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::CountTag>
    { return self().template get<&urd::record_of<V>::arg>(); }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::DistanceTag> || runir::kr::dl::NumericalBinaryTag<Tag>)
    { return self().template get<&urd::record_of<V>::lhs>(); }

    auto get_mid() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    { return self().template get<&urd::record_of<V>::mid>(); }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::DistanceTag> || runir::kr::dl::NumericalBinaryTag<Tag>)
    { return self().template get<&urd::record_of<V>::rhs>(); }

    auto get_value() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NumericalConstantTag>
    { return self().data().identifier; }
};
}  // namespace urd

#endif
