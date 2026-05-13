#ifndef RUNIR_KR_GP_DL_PARSER_HPP_
#define RUNIR_KR_GP_DL_PARSER_HPP_

#include "runir/kr/gp/dl/parser/parser.hpp"
#include "runir/kr/gp/repository.hpp"

#include <string>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::kr::gp::dl
{

PolicyView parse_policy(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);

}

#endif
