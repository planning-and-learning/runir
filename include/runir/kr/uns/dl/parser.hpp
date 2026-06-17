#ifndef RUNIR_KR_UNS_DL_PARSER_HPP_
#define RUNIR_KR_UNS_DL_PARSER_HPP_

#include "runir/kr/uns/dl/parser/parser.hpp"
#include "runir/kr/uns/repository.hpp"

#include <string>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::kr::uns::dl
{

ClassifierView parse_classifier(const std::string& description, tyr::formalism::planning::DomainView domain, Repository& repository);

}

#endif
