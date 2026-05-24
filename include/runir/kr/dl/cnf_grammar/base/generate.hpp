#ifndef RUNIR_KR_DL_CNF_GRAMMAR_BASE_GENERATE_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_BASE_GENERATE_HPP_

#include "runir/kr/dl/cnf_grammar/base/repository.hpp"
#include "runir/kr/dl/cnf_grammar/generate.hpp"

namespace runir::kr::dl::cnf_grammar::base
{

using GenerateResults = runir::kr::dl::cnf_grammar::GenerateResultsFor<runir::kr::BaseFamilyTag>;

template<tyr::planning::TaskKind Kind>
GenerateResults generate(GrammarView grammar,
                         const std::vector<tyr::planning::StateView<Kind>>& states,
                         ConstructorRepository& output_repository,
                         const GenerateOptions& options);

}  // namespace runir::kr::dl::cnf_grammar::base

#endif
