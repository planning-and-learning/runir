#ifndef RUNIR_KR_PS_UNS_DL_PARSER_HPP_
#define RUNIR_KR_PS_UNS_DL_PARSER_HPP_

#include "runir/kr/ps/uns/dl/parser/parser.hpp"
#include "runir/kr/ps/uns/repository.hpp"

#include <string>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::kr::ps::uns::dl
{

SketchView parse_sketch(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);

}

#endif
