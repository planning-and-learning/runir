#ifndef RUNIR_FORMATTER_HPP_
#define RUNIR_FORMATTER_HPP_

#include "runir/config.hpp"

#include <fmt/format.h>
#include <string>
#include <yggdrasil/formatting/formatter.hpp>

namespace runir
{

template<typename T>
std::string to_string(const T& element)
{
    return fmt::format("{}", element);
}

}  // namespace runir

#endif
