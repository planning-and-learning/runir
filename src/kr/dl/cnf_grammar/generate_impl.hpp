#ifndef RUNIR_KR_DL_CNF_GRAMMAR_GENERATE_IMPL_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_GENERATE_IMPL_HPP_

#include "generator.hpp"
#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/generate.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <stdexcept>
#include <tyr/formalism/repository.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <tyr/planning/states/state_view.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::dl::cnf_grammar
{

/// The states' task must be complete where this is instantiated: each instantiating source includes the task header of
/// its kind.
template<tyr::TaskKind Kind>
GenerateResultsFor generate(GrammarView grammar,
                            const urd::Store<urd::sequence<tyr::planning::StateView<Kind>>>& states,
                            runir::kr::dl::ConstructorRepository& output_repository,
                            runir::kr::dl::semantics::DenotationRepository& denotation_repository,
                            const GenerateOptions& options)
{
    const auto& planning_repository = output_repository.get_ptr<tyr::formalism::Repository>();
    const auto root = [](const tyr::formalism::Repository* layer)
    {
        while (layer->parent() != nullptr)
            layer = layer->parent();
        return layer;
    };
    if (root(&grammar.context().get<tyr::formalism::Repository>()) != root(planning_repository.get()))
        throw std::invalid_argument("generate requires grammar and output repository for the same planning domain");

    for (const auto& state : states)
    {
        const auto& task = state.context().get_task();
        if (task->get_repository() != planning_repository && task->get_domain().get_repository() != planning_repository)
            throw std::invalid_argument("generate requires states and output repository for the same planning domain");
        if (task->get_repository().get() != &denotation_repository.get<tyr::formalism::Repository>())
            throw std::invalid_argument("generate requires states and denotation repository for the same planning task");
    }

    return detail::Generator<Kind>(grammar, states, output_repository, denotation_repository, options).run();
}

}

#endif
