#ifndef RUNIR_KR_PS_BASE_DL_PARSER_HPP_
#define RUNIR_KR_PS_BASE_DL_PARSER_HPP_

#include "runir/kr/ps/base/repository.hpp"

#include <string>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::kr::ps::base::dl
{

SketchView parse_sketch(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);

}

#endif
