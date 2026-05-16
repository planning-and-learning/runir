#ifndef RUNIR_KR_GP_DL_DECLARATIONS_HPP_
#define RUNIR_KR_GP_DL_DECLARATIONS_HPP_

#include "runir/kr/gp/declarations.hpp"

namespace runir::kr::gp::dl
{

struct BooleanFeature
{
    static constexpr auto keyword = "boolean";
};

struct NumericalFeature
{
    static constexpr auto keyword = "numerical";
};

struct Positive
{
    static constexpr auto keyword = "positive";
};

struct Negative
{
    static constexpr auto keyword = "negative";
};

struct EqualZero
{
    static constexpr auto keyword = "equal_zero";
};

struct GreaterZero
{
    static constexpr auto keyword = "greater_zero";
};

struct Increases
{
    static constexpr auto keyword = "increases";
};

struct Decreases
{
    static constexpr auto keyword = "decreases";
};

struct Unchanged
{
    static constexpr auto keyword = "unchanged";
};

}  // namespace runir::kr::gp::dl

#endif
