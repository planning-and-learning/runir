#ifndef RUNIR_KR_PS_ICP_OBJECT_REFERENCE_HPP_
#define RUNIR_KR_PS_ICP_OBJECT_REFERENCE_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <compare>
#include <urd/elements/flat.hpp>
#include <urd/elements/index.hpp>
#include <yggdrasil/core/unsigned_integer.hpp>

namespace runir::kr::ps::icp
{

/// An argument's position: a nonnegative integer of its own type (no index: nothing resolves it).
struct ArgumentPosition : ygg::UnsignedInteger<ArgumentPosition>
{
    using UnsignedInteger::UnsignedInteger;
};

}

#endif
