#ifndef RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/ext/execution_canonicalization.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <tyr/planning/declarations.hpp>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
using ExecutionRepositoryTypes = ygg::TypeList<ModuleState<Kind>, CallStack, ProgramState<Kind>>;

template<tyr::TaskKind Kind>
using ExecutionBuilder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, ExecutionRepositoryTypes<Kind>>;

template<tyr::TaskKind Kind>
class ExecutionRepository : public ygg::formalism::SymbolRepositoryBase<ExecutionRepository<Kind>, ExecutionRepositoryTypes<Kind>>
{
    using Base = ygg::formalism::SymbolRepositoryBase<ExecutionRepository<Kind>, ExecutionRepositoryTypes<Kind>>;

    friend class ExecutionRepositoryFactory<Kind>;

private:
    ygg::uint_t m_index;
    tyr::planning::StateRepositoryPtr<Kind> m_state_repository;
    runir::kr::dl::semantics::DenotationRepositoryPtr m_denotation_repository;
    RepositoryPtr m_program_repository;

    ExecutionRepository(ygg::uint_t index,
                        tyr::planning::StateRepositoryPtr<Kind> state_repository,
                        runir::kr::dl::semantics::DenotationRepositoryPtr denotation_repository,
                        RepositoryPtr program_repository) :
        m_index(index),
        m_state_repository(std::move(state_repository)),
        m_denotation_repository(std::move(denotation_repository)),
        m_program_repository(std::move(program_repository))
    {
        assert(m_state_repository);
        assert(m_denotation_repository);
        assert(m_program_repository);
    }

public:
    ExecutionRepository(const ExecutionRepository&) = delete;
    ExecutionRepository& operator=(const ExecutionRepository&) = delete;
    ExecutionRepository(ExecutionRepository&&) = delete;
    ExecutionRepository& operator=(ExecutionRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    auto& get_state_repository() const noexcept { return *m_state_repository; }
    const auto& get_denotation_repository() const noexcept { return *m_denotation_repository; }
    const auto& get_program_repository() const noexcept { return *m_program_repository; }
    const auto& get_formalism_repository() const noexcept { return m_denotation_repository->get_formalism_repository(); }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ExecutionRepository, T>
    std::optional<ygg::View<ygg::Index<T>, ExecutionRepository>> find(const ygg::Data<T>& data) const noexcept
    {
        assert(is_canonical(data));
        return Base::find(data);
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ExecutionRepository, T>
    std::pair<ygg::View<ygg::Index<T>, ExecutionRepository>, bool> insert(ygg::Data<T>& data)
    {
        assert(is_canonical(data));
        return Base::insert(data);
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ExecutionRepository, T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const
    {
        assert(this->template is_local<T>(index));
        return this->template at_local<T>(index);
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ExecutionRepository, T>
    const ExecutionRepository& get_canonical_context(ygg::Index<T>) const noexcept
    {
        return *this;
    }
};

template<tyr::TaskKind Kind>
inline const ExecutionRepository<Kind>& get_repository(const ExecutionRepository<Kind>& context) noexcept
{
    return context;
}

template<tyr::TaskKind Kind>
class ExecutionRepositoryFactory
{
private:
    ygg::uint_t m_next_index = 0;

public:
    ExecutionRepositoryPtr<Kind> create_shared(tyr::planning::StateRepositoryPtr<Kind> state_repository,
                                               runir::kr::dl::semantics::DenotationRepositoryPtr denotation_repository,
                                               RepositoryPtr program_repository)
    {
        return ExecutionRepositoryPtr<Kind>(
            new ExecutionRepository<Kind>(m_next_index++, std::move(state_repository), std::move(denotation_repository), std::move(program_repository)));
    }
};

template<tyr::TaskKind Kind, typename T>
    requires ygg::formalism::SupportsSymbol<ExecutionRepository<Kind>, T>
void prepare_for_insert(ExecutionRepository<Kind>&, ygg::Data<T>& data)
{
    canonicalize(data);
}

using ygg::formalism::insert;

}  // namespace runir::kr::ps::ext

#endif
