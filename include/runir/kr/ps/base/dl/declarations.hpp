#ifndef RUNIR_KR_PS_BASE_DL_DECLARATIONS_HPP_
#define RUNIR_KR_PS_BASE_DL_DECLARATIONS_HPP_

#include "runir/kr/ps/declarations.hpp"

namespace runir::kr::ps::base::dl
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

}  // namespace runir::kr::ps::base::dl

#endif
