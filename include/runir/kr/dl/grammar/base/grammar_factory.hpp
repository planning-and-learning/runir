#ifndef RUNIR_KR_DL_GRAMMAR_BASE_GRAMMAR_FACTORY_HPP_
#define RUNIR_KR_DL_GRAMMAR_BASE_GRAMMAR_FACTORY_HPP_

#include "runir/kr/dl/grammar/base/parser.hpp"
#include "runir/kr/dl/grammar/base/repository.hpp"

#include <string>
#include <tyr/formalism/planning/domain_view.hpp>

namespace runir::kr::dl::grammar::base
{

enum class GrammarSpecification
{
    FRANCE_ET_AL_AAAI2021,
};

class GrammarFactory
{
public:
    static GrammarView create(GrammarSpecification specification, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository);

    static std::string create_description(GrammarSpecification specification, tyr::formalism::planning::DomainView domain);

    static GrammarView create_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, ConstructorRepository& repository);

    static std::string create_france_et_al_aaai2021_description(tyr::formalism::planning::DomainView domain);
};

}  // namespace runir::kr::dl::grammar::base

#endif
