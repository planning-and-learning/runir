#ifndef RUNIR_KR_PS_CONTROL_REPOSITORY_HPP_
#define RUNIR_KR_PS_CONTROL_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <cassert>
#include <cstddef>
#include <memory>
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

/// The control records of one task under a program family (FamilyTypes<Family>::ControlRepositoryTypes): those that hold
/// no planning state, so that they are the same whatever the task's kind, over the task's denotation and program
/// repositories.
template<ProgramFamilyTag Family>
class ControlRepository :
    public ygg::formalism::SymbolContext<ControlRepository<Family>, typename FamilyTypes<Family>::ControlRepositoryTypes>,
    public ygg::formalism::Nests<ControlRepository<Family>, ygg::formalism::Nested<const Repository<Family>, runir::kr::dl::semantics::DenotationRepository>>
{
    using Base = ygg::formalism::SymbolContext<ControlRepository<Family>, typename FamilyTypes<Family>::ControlRepositoryTypes>;
    using NestsBase =
        ygg::formalism::Nests<ControlRepository<Family>, ygg::formalism::Nested<const Repository<Family>, runir::kr::dl::semantics::DenotationRepository>>;
    using ProgramRepositoryPtr = std::shared_ptr<Repository<Family>>;

private:
    std::size_t m_index;

public:
    /// Over the task's denotation repository (interned into: registers, call arguments) and program repository (read
    /// only). Numbered by ygg::next_layer(), the process-wide counter, so that no two contexts share a number.
    ControlRepository(runir::kr::dl::semantics::DenotationRepositoryPtr denotation_repository, ProgramRepositoryPtr program_repository) :
        Base(),
        NestsBase(std::move(program_repository), std::move(denotation_repository)),
        m_index(ygg::next_layer())
    {
    }

    ControlRepository(const ControlRepository&) = delete;
    ControlRepository& operator=(const ControlRepository&) = delete;
    ControlRepository(ControlRepository&&) = delete;
    ControlRepository& operator=(ControlRepository&&) = delete;
    ~ControlRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    using Base::find;
    using Base::insert;
    using Base::view;
    using NestsBase::get;
    using NestsBase::get_ptr;
};

}  // namespace runir::kr::ps

#endif
