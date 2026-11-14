#ifndef RUNIR_KR_PS_EXECUTION_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXECUTION_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <cassert>
#include <cstddef>
#include <memory>
#include <tyr/planning/declarations.hpp>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::ps
{

/// The program states of one task under a program family: the family's execution records over the task's state,
/// denotation and program repositories.
template<ProgramFamilyTag Family, tyr::TaskKind Kind>
class ExecutionRepository :
    public ygg::formalism::SymbolContext<ExecutionRepository<Family, Kind>, typename FamilyTypes<Family>::template ExecutionRepositoryTypes<Kind>>
{
    using Base = ygg::formalism::SymbolContext<ExecutionRepository<Family, Kind>, typename FamilyTypes<Family>::template ExecutionRepositoryTypes<Kind>>;
    using ProgramRepositoryPtr = std::shared_ptr<Repository<Family>>;

private:
    std::size_t m_index;
    tyr::planning::StateRepositoryPtr<Kind> m_state_repository;
    runir::kr::dl::semantics::DenotationRepositoryPtr m_denotation_repository;
    ProgramRepositoryPtr m_program_repository;

public:
    /// Over the task's state, denotation and program repositories. Numbered by ygg::next_layer(), the process-wide
    /// counter, so that no two contexts share a number.
    ExecutionRepository(tyr::planning::StateRepositoryPtr<Kind> state_repository,
                        runir::kr::dl::semantics::DenotationRepositoryPtr denotation_repository,
                        ProgramRepositoryPtr program_repository) :
        m_index(ygg::next_layer()),
        m_state_repository(std::move(state_repository)),
        m_denotation_repository(std::move(denotation_repository)),
        m_program_repository(std::move(program_repository))
    {
        assert(m_state_repository);
        assert(m_denotation_repository);
        assert(m_program_repository);
    }
    ExecutionRepository(const ExecutionRepository&) = delete;
    ExecutionRepository& operator=(const ExecutionRepository&) = delete;
    ExecutionRepository(ExecutionRepository&&) = delete;
    ExecutionRepository& operator=(ExecutionRepository&&) = delete;
    ~ExecutionRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }
    [[nodiscard]] auto& get_state_repository() const noexcept { return *m_state_repository; }
    [[nodiscard]] const auto& get_denotation_repository() const noexcept { return *m_denotation_repository; }
    [[nodiscard]] const auto& get_program_repository() const noexcept { return *m_program_repository; }
    [[nodiscard]] const auto& get_formalism_repository() const noexcept { return m_denotation_repository->get_formalism_repository(); }
};

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
inline const ExecutionRepository<Family, Kind>& get_repository(const ExecutionRepository<Family, Kind>& context URD_LIFETIMEBOUND) noexcept
{ return context; }

/// Execution records are canonical as built.
template<ProgramFamilyTag Family, tyr::TaskKind Kind, typename T>
    requires ygg::formalism::SupportsSymbol<ExecutionRepository<Family, Kind>, T>
void prepare_for_insert(ExecutionRepository<Family, Kind>&, urd::data<T>&)
{
}

using ygg::formalism::insert;

}  // namespace runir::kr::ps

#endif
