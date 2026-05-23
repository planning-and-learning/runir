#ifndef RUNIR_KR_PS_BASE_DL_PARSER_PARSER_HPP_
#define RUNIR_KR_PS_BASE_DL_PARSER_PARSER_HPP_

#include "runir/kr/ps/base/dl/ast/ast.hpp"

#include <string>

namespace runir::kr::ps::base::dl::parser
{

template<runir::kr::FamilyTag Family>
ast::Sketch<Family> parse_sketch_ast(const std::string& description);

}

#endif
