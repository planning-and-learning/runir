#ifndef RUNIR_GRAMMAR_NUMERICAL_VIEW_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_VIEW_HPP_

#include "runir/kr/dl/grammar/numerical_data.hpp"

#include <concepts>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename Tag, typename V>
    requires runir::kr::dl::BaseNumericalConstructorTag<Tag>
struct view_mixin<runir::kr::dl::grammar::Numerical<Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::CountTag>
    { return self().template get<&urd::record_of<V>::arg>(); }

    auto get_lhs() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    { return self().template get<&urd::record_of<V>::lhs>(); }

    auto get_mid() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    { return self().template get<&urd::record_of<V>::mid>(); }

    auto get_rhs() const noexcept
        requires std::same_as<Tag, runir::kr::dl::DistanceTag>
    { return self().template get<&urd::record_of<V>::rhs>(); }
};
}  // namespace urd

#endif
