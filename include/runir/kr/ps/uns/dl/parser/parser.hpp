#ifndef RUNIR_KR_PS_UNS_DL_PARSER_PARSER_HPP_
#define RUNIR_KR_PS_UNS_DL_PARSER_PARSER_HPP_

#include "runir/kr/ps/uns/dl/ast/ast.hpp"

#include <string>

namespace runir::kr::ps::uns::dl::parser
{

ast::Sketch<runir::kr::UnsFamilyTag> parse_sketch_ast(const std::string& description);

}

#endif
