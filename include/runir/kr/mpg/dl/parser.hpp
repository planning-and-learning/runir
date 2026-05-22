#ifndef RUNIR_KR_MPG_DL_PARSER_HPP_
#define RUNIR_KR_MPG_DL_PARSER_HPP_

#include "runir/kr/mpg/repository.hpp"

#include <string>

namespace runir::kr::mpg::dl
{

PolicyView parse_policy(const std::string& description, Repository& repository);

}  // namespace runir::kr::mpg::dl

#endif
