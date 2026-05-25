#ifndef RUNIR_REPOSITORY_HPP_
#define RUNIR_REPOSITORY_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/datas.hpp"
#include "runir/kr/dl/semantics/views.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/formalism/symbol_repository.hpp>
#include <utility>
#include <vector>

namespace runir::kr::dl
{

template<FamilyTag Family>
struct RepositoryConstructorFamily
{
    template<typename Tag>
        requires FamilyConceptConstructorTag<Family, Tag>
    using Concept = FamilyConcept<Family, Tag>;

    template<typename Tag>
        requires FamilyRoleConstructorTag<Family, Tag>
    using Role = FamilyRole<Family, Tag>;

    template<typename Tag>
        requires FamilyBooleanConstructorTag<Family, Tag>
    using Boolean = FamilyBoolean<Family, Tag>;

    template<typename Tag>
        requires FamilyNumericalConstructorTag<Family, Tag>
    using Numerical = FamilyNumerical<Family, Tag>;

    template<CategoryTag Category>
    using Constructor = FamilyConstructor<Family, Category>;
};

template<FamilyTag Family>
using FamilyConceptTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Concept, FamilyConceptConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyRoleTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Role, FamilyRoleConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyBooleanTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Boolean, FamilyBooleanConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyNumericalTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Numerical, FamilyNumericalConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyConstructorTypes = tyr::MapTypeListT<RepositoryConstructorFamily<Family>::template Constructor, CategoryTags>;

template<FamilyTag Family>
using FamilyConstructorRepositoryTypes = tyr::ConcatTypeListsT<FamilyConceptTypes<Family>,
                                                               FamilyRoleTypes<Family>,
                                                               FamilyBooleanTypes<Family>,
                                                               FamilyNumericalTypes<Family>,
                                                               FamilyConstructorTypes<Family>>;

template<FamilyTag Family>
using FamilyConstructorSymbolRepository = tyr::ApplyTypeListT<tyr::formalism::SymbolRepository, FamilyConstructorRepositoryTypes<Family>>;

template<FamilyTag Family>
class BasicConstructorRepository
{
    template<FamilyTag>
    friend class BasicConstructorRepositoryFactory;

private:
    FamilyConstructorSymbolRepository<Family> m_symbol_repository;
    std::shared_ptr<const tyr::formalism::planning::Repository> m_planning_repository;
    size_t m_index;

    template<typename T>
    std::optional<tyr::View<tyr::Index<T>, BasicConstructorRepository>> find_with_hash(const tyr::Data<T>& data, size_t hash) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return tyr::View<tyr::Index<T>, BasicConstructorRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<tyr::View<tyr::Index<T>, BasicConstructorRepository>, bool> get_or_create_with_hash(tyr::Data<T>& data, size_t hash)
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return { tyr::View<tyr::Index<T>, BasicConstructorRepository>(*index, *this), false };

        const auto [index, success] = m_symbol_repository.template get_or_create_local_with_hash<T>(data, hash);
        return { tyr::View<tyr::Index<T>, BasicConstructorRepository>(index, *this), success };
    }

    BasicConstructorRepository(size_t index, std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository) :
        m_symbol_repository(nullptr),
        m_planning_repository(std::move(planning_repository)),
        m_index(index)
    {
        assert(m_planning_repository);
        clear();
    }

public:
    BasicConstructorRepository(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository& operator=(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository(BasicConstructorRepository&&) = delete;
    BasicConstructorRepository& operator=(BasicConstructorRepository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    const auto& get_planning_repository() const noexcept
    {
        assert(m_planning_repository);
        return *m_planning_repository;
    }
    const auto& get_planning_repository_ptr() const noexcept { return m_planning_repository; }

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    std::optional<tyr::View<tyr::Index<T>, BasicConstructorRepository>> find(const tyr::Data<T>& data) const noexcept
    {
        return find_with_hash<T>(data, FamilyConstructorSymbolRepository<Family>::template hash<T>(data));
    }

    template<typename T>
    std::pair<tyr::View<tyr::Index<T>, BasicConstructorRepository>, bool> get_or_create(tyr::Data<T>& data)
    {
        return get_or_create_with_hash<T>(data, FamilyConstructorSymbolRepository<Family>::template hash<T>(data));
    }

    template<typename T>
    const tyr::Data<T>& operator[](tyr::Index<T> index) const noexcept
    {
        assert(m_symbol_repository.template is_local<T>(index));
        return m_symbol_repository.template at_local<T>(index);
    }

    template<typename T>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<T>();
    }

    template<typename T>
    const BasicConstructorRepository& get_canonical_context(tyr::Index<T>) const noexcept
    {
        return *this;
    }
};

template<FamilyTag Family>
using ConstructorRepositoryFor = BasicConstructorRepository<Family>;

template<FamilyTag Family>
using ConstructorRepositoryPtrFor = std::shared_ptr<ConstructorRepositoryFor<Family>>;

template<FamilyTag Family>
class BasicConstructorRepositoryFactory
{
private:
    size_t m_next_index;

public:
    BasicConstructorRepositoryFactory() : m_next_index(0) {}

    ConstructorRepositoryFor<Family> create(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository)
    {
        return ConstructorRepositoryFor<Family>(m_next_index++, std::move(planning_repository));
    }

    ConstructorRepositoryPtrFor<Family> create_shared(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository)
    {
        return ConstructorRepositoryPtrFor<Family>(new ConstructorRepositoryFor<Family>(m_next_index++, std::move(planning_repository)));
    }
};

template<FamilyTag Family>
using ConstructorRepositoryFactoryFor = BasicConstructorRepositoryFactory<Family>;

template<FamilyTag Family, typename Tag>
    requires FamilyConceptConstructorTag<Family, Tag>
using FamilyConceptView = tyr::View<tyr::Index<FamilyConcept<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family, typename Tag>
    requires FamilyRoleConstructorTag<Family, Tag>
using FamilyRoleView = tyr::View<tyr::Index<FamilyRole<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family, typename Tag>
    requires FamilyBooleanConstructorTag<Family, Tag>
using FamilyBooleanView = tyr::View<tyr::Index<FamilyBoolean<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family, typename Tag>
    requires FamilyNumericalConstructorTag<Family, Tag>
using FamilyNumericalView = tyr::View<tyr::Index<FamilyNumerical<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family, CategoryTag Category>
using FamilyConstructorView = tyr::View<tyr::Index<FamilyConstructor<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family>
inline const ConstructorRepositoryFor<Family>& get_repository(const ConstructorRepositoryFor<Family>& repository) noexcept
{
    return repository;
}

template<FamilyTag Family>
inline ConstructorRepositoryFor<Family>& get_repository(ConstructorRepositoryFor<Family>& repository) noexcept
{
    return repository;
}

}

#endif
