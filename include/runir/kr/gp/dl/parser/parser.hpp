#ifndef RUNIR_KR_GP_DL_PARSER_PARSER_HPP_
#define RUNIR_KR_GP_DL_PARSER_PARSER_HPP_

#include "runir/kr/gp/dl/ast/ast.hpp"

#include <string>

namespace runir::kr::gp::dl::parser
{

ast::Policy parse_policy_ast(const std::string& description);

}

#endif
