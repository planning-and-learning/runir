#ifndef RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl::detail
{

template<FamilyTag Family, typename RepositoryTypes>
class ConstructorRepository
{
    friend class ConstructorRepositoryFactory<Family, RepositoryTypes>;

private:
    ygg::ApplyTypeListT<ygg::formalism::SymbolRepository, RepositoryTypes> m_symbol_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_planning_repository;
    size_t m_index;

    ConstructorRepository(size_t index, std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository) :
        m_symbol_repository(nullptr),
        m_planning_repository(std::move(planning_repository)),
        m_index(index)
    {
        assert(m_planning_repository);
        clear();
    }

public:
    using SymbolTypes = RepositoryTypes;

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

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
    std::optional<ygg::View<ygg::Index<T>, ConstructorRepository>> find(const ygg::Data<T>& data) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local<T>(data))
            return ygg::View<ygg::Index<T>, ConstructorRepository>(*index, *this);
        return std::nullopt;
    }

    /// Raw symbol interning. Use the language-specific free get_or_create() to canonicalize and prepare data.
    template<typename T>
        requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
    std::pair<ygg::View<ygg::Index<T>, ConstructorRepository>, bool> get_or_create(ygg::Data<T>& data)
    {
        const auto [index, created] = m_symbol_repository.template get_or_create_local<T>(data);
        return { ygg::View<ygg::Index<T>, ConstructorRepository>(index, *this), created };
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
    {
        assert(m_symbol_repository.template is_local<T>(index));
        return m_symbol_repository.template at_local<T>(index);
    }

    template<typename T>
        requires ygg::formalism::SupportsSymbol<ConstructorRepository, T>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<T>();
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
