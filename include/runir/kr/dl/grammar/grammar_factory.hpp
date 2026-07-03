#ifndef RUNIR_KR_DL_GRAMMAR_GRAMMAR_FACTORY_HPP_
#define RUNIR_KR_DL_GRAMMAR_GRAMMAR_FACTORY_HPP_

#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/parser.hpp"

#include <string>
#include <tyr/formalism/planning/domain_view.hpp>

namespace runir::kr::dl::grammar
{

enum class GrammarSpecification
{
    FRANCE_ET_AL_AAAI2021,
};

class GrammarFactory
{
public:
    static FamilyGrammarView<runir::kr::BaseFamilyTag> create(GrammarSpecification specification,
                                                              tyr::formalism::planning::DomainView domain,
                                                              ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository);

    static std::string create_description(GrammarSpecification specification, tyr::formalism::planning::DomainView domain);

    static FamilyGrammarView<runir::kr::BaseFamilyTag> create_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain,
                                                                                    ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository);

    static std::string create_france_et_al_aaai2021_description(tyr::formalism::planning::DomainView domain);
};

}  // namespace runir::kr::dl::grammar

#endif
