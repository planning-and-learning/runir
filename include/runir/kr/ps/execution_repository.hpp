#ifndef RUNIR_KR_PS_EXECUTION_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXECUTION_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/control_repository.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <tyr/planning/declarations.hpp>
#include <urd/config.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/flat.hpp>
#include <urd/elements/view.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::ps
{

/// The program states of one task of kind Kind under a program family: the family's execution records over the task's
/// state repository and its control repository (and, through it, the task's denotation and program repositories).
template<ProgramFamilyTag Family, tyr::TaskKind Kind>
class ExecutionRepository :
    public ygg::formalism::SymbolContext<ExecutionRepository<Family, Kind>, typename FamilyTypes<Family>::template ExecutionRepositoryTypes<Kind>>
{
    using Base = ygg::formalism::SymbolContext<ExecutionRepository<Family, Kind>, typename FamilyTypes<Family>::template ExecutionRepositoryTypes<Kind>>;

private:
    std::size_t m_index;
    tyr::planning::StateRepositoryPtr<Kind> m_state_repository;
    ControlRepositoryPtr<Family> m_control_repository;

public:
    /// Over the task's state and control repositories. Numbered by ygg::next_layer(), the process-wide counter, so that
    /// no two contexts share a number.
    ExecutionRepository(tyr::planning::StateRepositoryPtr<Kind> state_repository, ControlRepositoryPtr<Family> control_repository) :
        m_index(ygg::next_layer()),
        m_state_repository(std::move(state_repository)),
        m_control_repository(std::move(control_repository))
    {
        assert(m_state_repository);
        assert(m_control_repository);
    }

    ExecutionRepository(const ExecutionRepository&) = delete;
    ExecutionRepository& operator=(const ExecutionRepository&) = delete;
    ExecutionRepository(ExecutionRepository&&) = delete;
    ExecutionRepository& operator=(ExecutionRepository&&) = delete;
    ~ExecutionRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    [[nodiscard]] auto& get_state_repository() const noexcept { return *m_state_repository; }

    [[nodiscard]] auto& get_control_repository() const noexcept { return *m_control_repository; }

    [[nodiscard]] const auto& get_denotation_repository() const noexcept { return m_control_repository->get_denotation_repository(); }

    [[nodiscard]] const auto& get_program_repository() const noexcept { return m_control_repository->get_program_repository(); }

    [[nodiscard]] const auto& get_formalism_repository() const noexcept { return m_control_repository->get_formalism_repository(); }

    using Base::view;

    /// A program symbol, a denotation or a state as its view in the repository that holds it: so that urd resolves every
    /// member of an execution record through this context (urd's resolves contract).
    template<typename T>
        requires ygg::formalism::SupportsSymbol<Repository<Family>, T>
    [[nodiscard]] auto view(urd::index<T> index) const
    { return get_program_repository().view(index); }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
    [[nodiscard]] auto view(urd::index<T> index) const
    { return get_denotation_repository().view(index); }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ControlRepository<Family>, T>
    [[nodiscard]] auto view(urd::index<T> index) const
    { return get_control_repository().view(index); }

    [[nodiscard]] tyr::planning::PackedStateView<Kind> view(urd::index<tyr::planning::State<Kind>> index) const { return get_state_repository().view(index); }

    /// A built child of an execution record (urd::child) as the view the repository that holds its kind makes of it, as
    /// for an interned child: a program symbol's, a denotation's, a formalism record's (a register's values) or a state's.
    template<typename T>
    [[nodiscard]] auto view(const urd::builder<T, Repository<Family>>& built URD_LIFETIMEBOUND) const
    { return urd::resolve(built, get_program_repository()); }

    template<typename T>
    [[nodiscard]] auto view(const urd::builder<T, runir::kr::dl::semantics::DenotationRepository>& built URD_LIFETIMEBOUND) const
    { return urd::resolve(built, get_denotation_repository()); }

    template<typename T>
    [[nodiscard]] auto view(const urd::builder<T, ControlRepository<Family>>& built URD_LIFETIMEBOUND) const
    { return urd::resolve(built, get_control_repository()); }

    [[nodiscard]] tyr::planning::StateBuilderView<Kind>
    view(const urd::builder<tyr::planning::State<Kind>, tyr::planning::StateRepository<Kind>>& built URD_LIFETIMEBOUND) const
    { return urd::resolve(built, get_state_repository()); }

    using Base::find;
    using Base::insert;

    /// The records of an execution record's children that other repositories hold, interned there (the match-tree
    /// rule): a control or a denotation repository's records in it (through the control repository), from a builder or
    /// a record of its own, with scratch records from `storage`; a planning state built for a program state registered
    /// in the state repository (its key).
    template<typename T, typename C, ygg::formalism::IsBuilderStorage Storage>
        requires(std::same_as<C, ControlRepository<Family>> || std::same_as<C, runir::kr::dl::semantics::DenotationRepository>)
    decltype(auto) insert(const urd::builder<T, C>& built, Storage& storage)
    { return m_control_repository->insert(built, storage); }

    template<typename T>
        requires(ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
                 || ygg::formalism::SupportsSymbol<ControlRepository<Family>, T>)
    auto insert(urd::data<T>& record)
    { return m_control_repository->insert(record); }

    template<ygg::formalism::IsBuilderStorage Storage>
    urd::index<tyr::planning::State<Kind>> insert(const urd::builder<tyr::planning::State<Kind>, tyr::planning::StateRepository<Kind>>& built,
                                                  Storage& /*storage*/)
    { return m_state_repository->register_extended_state(urd::resolve(built, *m_state_repository)).pack().index(); }

    /// Likewise for finding, inserting nothing (a built planning state is not looked up: its program state is not found).
    template<typename T, typename C, ygg::formalism::IsBuilderStorage Storage>
        requires(std::same_as<C, ControlRepository<Family>> || std::same_as<C, runir::kr::dl::semantics::DenotationRepository>)
    auto find(const urd::builder<T, C>& built, Storage& storage)
    { return m_control_repository->find(built, storage); }

    template<typename T>
        requires(ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
                 || ygg::formalism::SupportsSymbol<ControlRepository<Family>, T>)
    auto find(const urd::data<T>& record)
    { return m_control_repository->find(record); }
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
