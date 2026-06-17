#ifndef RUNIR_KR_UNS_DL_PARSER_PARSER_HPP_
#define RUNIR_KR_UNS_DL_PARSER_PARSER_HPP_

#include "runir/kr/uns/dl/ast/ast.hpp"

#include <string>

namespace runir::kr::uns::dl::parser
{

ast::Classifier parse_classifier_ast(const std::string& description);

}

#endif
