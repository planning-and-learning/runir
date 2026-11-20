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
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::ps
{

/// The control records of one task under a program family (FamilyTypes<Family>::ControlRepositoryTypes): those that hold
/// no planning state, so that they are the same whatever the task's kind, over the task's denotation and program
/// repositories.
template<ProgramFamilyTag Family>
class ControlRepository : public ygg::formalism::SymbolContext<ControlRepository<Family>, typename FamilyTypes<Family>::ControlRepositoryTypes>
{
    using Base = ygg::formalism::SymbolContext<ControlRepository<Family>, typename FamilyTypes<Family>::ControlRepositoryTypes>;
    using ProgramRepositoryPtr = std::shared_ptr<Repository<Family>>;

private:
    std::size_t m_index;
    runir::kr::dl::semantics::DenotationRepositoryPtr m_denotation_repository;
    ProgramRepositoryPtr m_program_repository;

public:
    /// Over the task's denotation and program repositories. Numbered by ygg::next_layer(), the process-wide counter, so
    /// that no two contexts share a number.
    ControlRepository(runir::kr::dl::semantics::DenotationRepositoryPtr denotation_repository, ProgramRepositoryPtr program_repository) :
        m_index(ygg::next_layer()),
        m_denotation_repository(std::move(denotation_repository)),
        m_program_repository(std::move(program_repository))
    {
        assert(m_denotation_repository);
        assert(m_program_repository);
    }

    ControlRepository(const ControlRepository&) = delete;
    ControlRepository& operator=(const ControlRepository&) = delete;
    ControlRepository(ControlRepository&&) = delete;
    ControlRepository& operator=(ControlRepository&&) = delete;
    ~ControlRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    [[nodiscard]] const auto& get_denotation_repository() const noexcept { return *m_denotation_repository; }

    [[nodiscard]] const auto& get_program_repository() const noexcept { return *m_program_repository; }

    [[nodiscard]] const auto& get_formalism_repository() const noexcept { return m_denotation_repository->get_formalism_repository(); }

    using Base::view;

    /// A program symbol or a denotation as its view in the repository that holds it: so that urd resolves every member
    /// of a control record through this context (urd's resolves contract).
    template<typename T>
        requires ygg::formalism::SupportsSymbol<Repository<Family>, T>
    [[nodiscard]] auto view(urd::index<T> index) const
    { return get_program_repository().view(index); }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
    [[nodiscard]] auto view(urd::index<T> index) const
    { return get_denotation_repository().view(index); }

    /// A built child of a control record (urd::child) as the view the repository that holds its kind makes of it, as for
    /// an interned child.
    template<typename T>
    [[nodiscard]] auto view(const urd::builder<T, Repository<Family>>& built URD_LIFETIMEBOUND) const
    { return urd::resolve(built, get_program_repository()); }

    template<typename T>
    [[nodiscard]] auto view(const urd::builder<T, runir::kr::dl::semantics::DenotationRepository>& built URD_LIFETIMEBOUND) const
    { return urd::resolve(built, get_denotation_repository()); }

    /// A held child of a control record (a record not interned yet, urd::child) as the view the repository that holds
    /// its kind makes of it, as for a built one.
    template<typename T>
        requires ygg::formalism::SupportsSymbol<Repository<Family>, T>
    [[nodiscard]] auto view(const urd::data<T>& record URD_LIFETIMEBOUND) const
    { return urd::resolve(record, get_program_repository()); }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
    [[nodiscard]] auto view(const urd::data<T>& record URD_LIFETIMEBOUND) const
    { return urd::resolve(record, get_denotation_repository()); }

    using Base::find;
    using Base::insert;

    /// The records of a control record's children that the denotation repository holds (registers, call arguments),
    /// interned there (the match-tree rule), from a builder or a record of its own, with scratch records from `storage`.
    template<typename T, ygg::formalism::IsBuilderStorage Storage>
        requires ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
    decltype(auto) insert(const urd::builder<T, runir::kr::dl::semantics::DenotationRepository>& built, Storage& storage)
    { return m_denotation_repository->insert(built, storage); }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
    auto insert(urd::data<T>& record)
    { return ygg::formalism::insert(*m_denotation_repository, record); }

    /// Likewise for finding, inserting nothing.
    template<typename T, ygg::formalism::IsBuilderStorage Storage>
        requires ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
    auto find(const urd::builder<T, runir::kr::dl::semantics::DenotationRepository>& built, Storage& storage)
    { return m_denotation_repository->find(built, storage); }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<runir::kr::dl::semantics::DenotationRepository, T>
    auto find(const urd::data<T>& record)
    { return m_denotation_repository->find(record); }
};

template<ProgramFamilyTag Family>
inline const ControlRepository<Family>& get_repository(const ControlRepository<Family>& context URD_LIFETIMEBOUND) noexcept
{ return context; }

/// Control records are canonical as built.
template<ProgramFamilyTag Family, typename T>
    requires ygg::formalism::SupportsSymbol<ControlRepository<Family>, T>
void prepare_for_insert(ControlRepository<Family>&, urd::data<T>&)
{
}

using ygg::formalism::insert;

}  // namespace runir::kr::ps

#endif
