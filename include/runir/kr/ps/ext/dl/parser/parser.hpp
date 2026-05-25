#ifndef RUNIR_KR_PS_EXT_DL_PARSER_PARSER_HPP_
#define RUNIR_KR_PS_EXT_DL_PARSER_PARSER_HPP_

#include "runir/kr/ps/ext/dl/ast/module_ast.hpp"

#include <string>

namespace runir::kr::ps::ext::dl::parser
{

ast::Module parse_module_ast(const std::string& description);

}  // namespace runir::kr::ps::ext::dl::parser

#endif
