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
#include <yggdrasil/formalism/nested.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::ps
{

/// The program states of one task of kind Kind under a program family: the family's execution records over the task's
/// state repository and its control repository (and, through it, the task's denotation and program repositories).
template<ProgramFamilyTag Family, tyr::TaskKind Kind>
class ExecutionRepository :
    public ygg::formalism::SymbolContext<ExecutionRepository<Family, Kind>, typename FamilyTypes<Family>::template ExecutionRepositoryTypes<Kind>>,
    public ygg::formalism::Nests<ExecutionRepository<Family, Kind>, ygg::formalism::Nested<tyr::planning::StateRepository<Kind>, ControlRepository<Family>>>
{
    using Base = ygg::formalism::SymbolContext<ExecutionRepository<Family, Kind>, typename FamilyTypes<Family>::template ExecutionRepositoryTypes<Kind>>;
    using NestsBase =
        ygg::formalism::Nests<ExecutionRepository<Family, Kind>, ygg::formalism::Nested<tyr::planning::StateRepository<Kind>, ControlRepository<Family>>>;

private:
    std::size_t m_index;

public:
    /// Over the task's state and control repositories (both interned into: a planning state built for a program state,
    /// and through the control repository its records and the denotations). Numbered by ygg::next_layer(), the
    /// process-wide counter, so that no two contexts share a number.
    ExecutionRepository(tyr::planning::StateRepositoryPtr<Kind> state_repository, ControlRepositoryPtr<Family> control_repository) :
        Base(),
        NestsBase(std::move(state_repository), std::move(control_repository)),
        m_index(ygg::next_layer())
    {
    }

    ExecutionRepository(const ExecutionRepository&) = delete;
    ExecutionRepository& operator=(const ExecutionRepository&) = delete;
    ExecutionRepository(ExecutionRepository&&) = delete;
    ExecutionRepository& operator=(ExecutionRepository&&) = delete;
    ~ExecutionRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    using Base::find;
    using Base::insert;
    using Base::view;
    using NestsBase::get;
    using NestsBase::get_ptr;
};

template<ProgramFamilyTag Family, tyr::TaskKind Kind>
inline const ExecutionRepository<Family, Kind>& get_repository(const ExecutionRepository<Family, Kind>& context URD_LIFETIMEBOUND) noexcept
{ return context; }

}  // namespace runir::kr::ps

#endif
