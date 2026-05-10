#ifndef RUNIR_COMMON_FORMATTER_HPP_
#define RUNIR_COMMON_FORMATTER_HPP_

#include "runir/common/config.hpp"

#include <fmt/format.h>
#include <string>
#include <tyr/common/formatter.hpp>

namespace runir
{

template<typename T>
std::string to_string(const T& element)
{
    return fmt::format("{}", element);
}

}  // namespace runir

#endif
