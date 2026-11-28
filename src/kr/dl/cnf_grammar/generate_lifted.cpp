#include "generate_impl.hpp"
#include "runir/kr/dl/cnf_grammar/generate.hpp"

#include <tyr/planning/tasks/lifted/task.hpp>

namespace runir::kr::dl::cnf_grammar
{

template GenerateResultsFor generate<tyr::LiftedTag>(GrammarView,
                                                     const urd::Store<urd::sequence<tyr::planning::StateView<tyr::LiftedTag>>>&,
                                                     runir::kr::dl::ConstructorRepository&,
                                                     runir::kr::dl::semantics::DenotationRepository&,
                                                     const GenerateOptions&);

}
