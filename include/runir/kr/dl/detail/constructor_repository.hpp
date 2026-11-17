#ifndef RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <boost/mp11.hpp>
#include <cassert>
#include <memory>
#include <tyr/formalism/planning/declarations.hpp>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::dl::detail
{

template<typename RepositoryTypes>
class BasicConstructorRepository : public ygg::formalism::SymbolContext<BasicConstructorRepository<RepositoryTypes>, RepositoryTypes>
{
    using Base = ygg::formalism::SymbolContext<BasicConstructorRepository<RepositoryTypes>, RepositoryTypes>;

private:
    std::shared_ptr<const tyr::formalism::planning::Repository> m_planning_repository;
    size_t m_index;

public:
    /// Over the planning repository whose symbols its records refer to. Numbered by ygg::next_layer(), the process-wide
    /// counter, so that no two contexts share a number.
    explicit BasicConstructorRepository(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository) :
        m_planning_repository(std::move(planning_repository)),
        m_index(ygg::next_layer())
    {
        assert(m_planning_repository);
        this->clear();
    }

    BasicConstructorRepository(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository& operator=(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository(BasicConstructorRepository&&) = delete;
    BasicConstructorRepository& operator=(BasicConstructorRepository&&) = delete;
    ~BasicConstructorRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    [[nodiscard]] const tyr::formalism::planning::Repository& get_planning_repository() const noexcept
    {
        assert(m_planning_repository);
        // Dereference the raw pointer: operator* on the shared_ptr would complete the repository type for its lookup.
        return *m_planning_repository.get();
    }

    [[nodiscard]] const auto& get_planning_repository_ptr() const noexcept { return m_planning_repository; }

    using Base::view;

    /// A planning symbol (a predicate, an object) as its view in the planning repository, which holds it: so that urd
    /// resolves every member of a record through this context (urd's resolves contract).
    template<typename T>
        requires ygg::formalism::SupportsSymbol<tyr::formalism::planning::Repository, T>
    [[nodiscard]] urd::index_view<T, tyr::formalism::planning::Repository> view(urd::index<T> index) const
    { return get_planning_repository().view(index); }
};

template<typename RepositoryTypes>
inline const BasicConstructorRepository<RepositoryTypes>&
get_repository(const BasicConstructorRepository<RepositoryTypes>& repository URD_LIFETIMEBOUND) noexcept
{ return repository; }

template<typename RepositoryTypes>
inline BasicConstructorRepository<RepositoryTypes>& get_repository(BasicConstructorRepository<RepositoryTypes>& repository) noexcept
{ return repository; }

}  // namespace runir::kr::dl::detail

#endif
