#ifndef RUNIR_KR_GP_DL_DECLARATIONS_HPP_
#define RUNIR_KR_GP_DL_DECLARATIONS_HPP_

#include "runir/kr/gp/declarations.hpp"

namespace runir::kr::gp::dl
{

struct BooleanFeatureTag
{
};

struct NumericalFeatureTag
{
};

struct PositiveTag
{
};

struct NegativeTag
{
};

struct EqualZeroTag
{
};

struct GreaterZeroTag
{
};

struct BecomesTrueTag
{
};

struct BecomesFalseTag
{
};

struct IncreasesTag
{
};

struct DecreasesTag
{
};

struct UnchangedTag
{
};

}  // namespace runir::kr::gp::dl

#endif
