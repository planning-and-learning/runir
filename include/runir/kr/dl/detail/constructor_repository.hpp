#ifndef RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cassert>
#include <memory>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl::detail
{

template<FamilyTag Family, typename RepositoryTypes>
class ConstructorRepository : public ygg::formalism::SymbolRepositoryBase<ConstructorRepository<Family, RepositoryTypes>, RepositoryTypes>
{
    using Base = ygg::formalism::SymbolRepositoryBase<ConstructorRepository<Family, RepositoryTypes>, RepositoryTypes>;

    friend class ConstructorRepositoryFactory<Family, RepositoryTypes>;

private:
    std::shared_ptr<const tyr::formalism::planning::Repository> m_planning_repository;
    size_t m_index;

    ConstructorRepository(size_t index, std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository) :
        m_planning_repository(std::move(planning_repository)),
        m_index(index)
    {
        assert(m_planning_repository);
        this->clear();
    }

public:
    ConstructorRepository(const ConstructorRepository&) = delete;
    ConstructorRepository& operator=(const ConstructorRepository&) = delete;
    ConstructorRepository(ConstructorRepository&&) = delete;
    ConstructorRepository& operator=(ConstructorRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    const auto& get_planning_repository() const noexcept
    {
        assert(m_planning_repository);
        return *m_planning_repository;
    }
    const auto& get_planning_repository_ptr() const noexcept { return m_planning_repository; }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
    {
        assert(this->template is_local<T>(index));
        return this->template at_local<T>(index);
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
    const ConstructorRepository& get_canonical_context(ygg::Index<T>) const noexcept
    {
        return *this;
    }
};

template<FamilyTag Family, typename RepositoryTypes>
class ConstructorRepositoryFactory
{
private:
    size_t m_next_index;

public:
    ConstructorRepositoryFactory() : m_next_index(0) {}

    std::shared_ptr<ConstructorRepository<Family, RepositoryTypes>> create(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository)
    {
        return std::shared_ptr<ConstructorRepository<Family, RepositoryTypes>>(
            new ConstructorRepository<Family, RepositoryTypes>(m_next_index++, std::move(planning_repository)));
    }
};

template<FamilyTag Family, typename RepositoryTypes>
inline const ConstructorRepository<Family, RepositoryTypes>& get_repository(const ConstructorRepository<Family, RepositoryTypes>& repository) noexcept
{
    return repository;
}

template<FamilyTag Family, typename RepositoryTypes>
inline ConstructorRepository<Family, RepositoryTypes>& get_repository(ConstructorRepository<Family, RepositoryTypes>& repository) noexcept
{
    return repository;
}

}  // namespace runir::kr::dl::detail

#endif
