#ifndef RUNIR_KR_UNS_DL_PARSER_HPP_
#define RUNIR_KR_UNS_DL_PARSER_HPP_

#include "runir/kr/uns/declarations.hpp"

#include <string>
#include <tyr/formalism/planning/declarations.hpp>

namespace runir::kr::uns::dl
{

ClassifierView parse_classifier(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);

}

#endif
