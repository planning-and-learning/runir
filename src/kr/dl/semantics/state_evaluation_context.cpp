#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_policy.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"

#include <stdexcept>
#include <string>

namespace runir::kr::dl::semantics::detail
{

EvaluationWorkspace& get_workspace(Builder& builder) noexcept { return builder.get_workspace(); }

ygg::database::Workspace& get_database_workspace(EvaluationWorkspace& workspace) noexcept { return workspace.get_database_workspace(); }

const tyr::formalism::planning::Repository* get_formalism_repository(const DenotationRepository& repository) noexcept
{
    return &repository.get_formalism_repository();
}

const ygg::Data<RegisterValues>& empty_register_values() noexcept
{
    static const auto values = ygg::Data<RegisterValues>();
    return values;
}

void throw_unbound_register(size_t index, size_t count)
{
    throw std::invalid_argument("DL evaluation: register " + std::to_string(index) + " is not bound; the evaluation context binds "
                                + std::to_string(count) + " registers of its category (only programs bind registers).");
}

void throw_unbound_argument(size_t index, size_t count)
{
    throw std::invalid_argument("DL evaluation: argument " + std::to_string(index) + " is not bound; the evaluation context binds "
                                + std::to_string(count) + " arguments of its category (only ext programs bind arguments).");
}

void throw_unbound_arguments()
{
    throw std::invalid_argument("DL evaluation: the evaluation context binds no call arguments (only ext programs bind arguments).");
}

}  // namespace runir::kr::dl::semantics::detail
