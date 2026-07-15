#ifndef RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/ext/execution_canonicalization.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <tyr/planning/state_repository.hpp>
#include <utility>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
using ExecutionSymbolRepository = ygg::formalism::SymbolRepository<RegisterValues, CallArguments, CallStack, ExecutionState<Kind>>;

template<tyr::planning::TaskKind Kind>
class ExecutionRepository
{
    friend class ExecutionRepositoryFactory<Kind>;

private:
    ygg::uint_t m_index;
    ExecutionSymbolRepository<Kind> m_symbol_repository;
    tyr::planning::StateRepositoryPtr<Kind> m_state_repository;
    runir::kr::dl::semantics::DenotationRepositoryPtr m_denotation_repository;
    RepositoryPtr m_program_repository;

    ExecutionRepository(ygg::uint_t index,
                        tyr::planning::StateRepositoryPtr<Kind> state_repository,
                        runir::kr::dl::semantics::DenotationRepositoryPtr denotation_repository,
                        RepositoryPtr program_repository) :
        m_index(index),
        m_symbol_repository(nullptr),
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
    const auto& get_formalism_repository() const noexcept { return m_program_repository->get_dl_repository().get_planning_repository(); }
    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    std::optional<ygg::View<ygg::Index<T>, ExecutionRepository>> find(const ygg::Data<T>& data) const noexcept
    {
        assert(is_canonical(data));
        if (auto index = m_symbol_repository.template find_local<T>(data))
            return ygg::View<ygg::Index<T>, ExecutionRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<ygg::View<ygg::Index<T>, ExecutionRepository>, bool> get_or_create(ygg::Data<T>& data)
    {
        canonicalize(data);
        assert(is_canonical(data));
        const auto [index, success] = m_symbol_repository.template get_or_create_local<T>(data);
        return { ygg::View<ygg::Index<T>, ExecutionRepository>(index, *this), success };
    }

    template<typename T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const
    {
        assert(m_symbol_repository.template is_local<T>(index));
        return m_symbol_repository.template at_local<T>(index);
    }

    template<typename T>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<T>();
    }
};

template<tyr::planning::TaskKind Kind>
inline const ExecutionRepository<Kind>& get_repository(const ExecutionRepository<Kind>& context) noexcept
{
    return context;
}

template<tyr::planning::TaskKind Kind>
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

}  // namespace runir::kr::ps::ext

#endif
