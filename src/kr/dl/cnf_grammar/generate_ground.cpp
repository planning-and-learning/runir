#include "generate_impl.hpp"
#include "runir/kr/dl/cnf_grammar/generate.hpp"

#include <tyr/planning/tasks/ground/task.hpp>

namespace runir::kr::dl::cnf_grammar
{

template GenerateResultsFor generate<tyr::GroundTag>(GrammarView,
                                                     const urd::Store<urd::sequence<tyr::planning::StateView<tyr::GroundTag>>>&,
                                                     runir::kr::dl::ConstructorRepository&,
                                                     runir::kr::dl::semantics::DenotationRepository&,
                                                     const GenerateOptions&);

}
