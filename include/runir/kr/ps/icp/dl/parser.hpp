#ifndef RUNIR_KR_PS_ICP_DL_PARSER_HPP_
#define RUNIR_KR_PS_ICP_DL_PARSER_HPP_

#include "runir/kr/ps/ext/dl/parser.hpp"
#include "runir/kr/ps/icp/declarations.hpp"

namespace runir::kr::ps::icp::dl
{

using runir::kr::ps::ext::dl::parse_boolean;
using runir::kr::ps::ext::dl::parse_concept;
using runir::kr::ps::ext::dl::parse_numerical;
using runir::kr::ps::ext::dl::parse_role;

ModuleView parse_module(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);
ProgramView parse_program(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);

}  // namespace runir::kr::ps::icp::dl

#endif
