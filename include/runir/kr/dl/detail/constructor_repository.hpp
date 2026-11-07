#ifndef RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cassert>
#include <memory>
#include <tyr/formalism/planning/declarations.hpp>
#include <utility>
#include <urd/config.hpp>
#include <boost/mp11.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl::detail
{

template<typename RepositoryTypes>
class BasicConstructorRepository : public ygg::formalism::SymbolRepositoryBase<BasicConstructorRepository<RepositoryTypes>, RepositoryTypes>
{
    using Base = ygg::formalism::SymbolRepositoryBase<BasicConstructorRepository<RepositoryTypes>, RepositoryTypes>;

    friend class BasicConstructorRepositoryFactory<RepositoryTypes>;

private:
    std::shared_ptr<const tyr::formalism::planning::Repository> m_planning_repository;
    size_t m_index;

    BasicConstructorRepository(size_t index, std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository) :
        m_planning_repository(std::move(planning_repository)),
        m_index(index)
    {
        assert(m_planning_repository);
        this->clear();
    }

public:
    BasicConstructorRepository(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository& operator=(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository(BasicConstructorRepository&&) = delete;
    BasicConstructorRepository& operator=(BasicConstructorRepository&&) = delete;
    ~BasicConstructorRepository() = default;

    [[nodiscard]] const auto& get_index() const noexcept { return m_index; }
    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }
    [[nodiscard]] const tyr::formalism::planning::Repository& get_planning_repository() const noexcept
    {
        assert(m_planning_repository);
        // Dereference the raw pointer: operator* on the shared_ptr would complete the repository type for its lookup.
        return *m_planning_repository.get();
    }
    [[nodiscard]] const auto& get_planning_repository_ptr() const noexcept { return m_planning_repository; }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<BasicConstructorRepository, T>
    const urd::data<T>& operator[](urd::index<T> index) const noexcept
    {
        assert(this->template is_local<T>(index));
        return this->template local<T>(index);
    }
};

template<typename RepositoryTypes>
class BasicConstructorRepositoryFactory
{
private:
    size_t m_next_index = 0;

public:
    BasicConstructorRepositoryFactory() = default;

    /// Defined in detail/constructor_repository_impl.hpp: an in-class definition would construct and destroy every
    /// symbol pool in each caller (template rule 7).
    std::shared_ptr<BasicConstructorRepository<RepositoryTypes>> create(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository);
};

template<typename RepositoryTypes>
inline const BasicConstructorRepository<RepositoryTypes>&
get_repository(const BasicConstructorRepository<RepositoryTypes>& repository URD_LIFETIMEBOUND) noexcept
{ return repository; }

template<typename RepositoryTypes>
inline BasicConstructorRepository<RepositoryTypes>& get_repository(BasicConstructorRepository<RepositoryTypes>& repository) noexcept
{ return repository; }

}  // namespace runir::kr::dl::detail

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/dl/detail/constructor_repository_impl.hpp"
#endif

#endif
