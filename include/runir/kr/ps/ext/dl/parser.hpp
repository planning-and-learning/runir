#ifndef RUNIR_KR_PS_EXT_DL_PARSER_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_HPP_

#include "runir/kr/ps/ext/repository.hpp"

#include <string>

namespace runir::kr::ps::ext::dl
{

ModuleView parse_module(const std::string& description, Repository& repository);

}  // namespace runir::kr::ps::ext::dl

#endif
