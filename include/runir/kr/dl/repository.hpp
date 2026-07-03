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
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/type_list.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

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
using FamilyConceptTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Concept, FamilyConceptConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyRoleTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Role, FamilyRoleConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyBooleanTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Boolean, FamilyBooleanConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyNumericalTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Numerical, FamilyNumericalConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyConstructorTypes = ygg::MapTypeListT<RepositoryConstructorFamily<Family>::template Constructor, CategoryTags>;

template<FamilyTag Family>
using FamilyConstructorRepositoryTypes = ygg::ConcatTypeListsT<FamilyConceptTypes<Family>,
                                                               FamilyRoleTypes<Family>,
                                                               FamilyBooleanTypes<Family>,
                                                               FamilyNumericalTypes<Family>,
                                                               FamilyConstructorTypes<Family>>;

template<FamilyTag Family>
using FamilyConstructorSymbolRepository = ygg::ApplyTypeListT<ygg::formalism::SymbolRepository, FamilyConstructorRepositoryTypes<Family>>;

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
    std::optional<ygg::View<ygg::Index<T>, BasicConstructorRepository>> find_with_hash(const ygg::Data<T>& data, size_t hash) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return ygg::View<ygg::Index<T>, BasicConstructorRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<ygg::View<ygg::Index<T>, BasicConstructorRepository>, bool> get_or_create_with_hash(ygg::Data<T>& data, size_t hash)
    {
        if (auto index = m_symbol_repository.template find_local_with_hash<T>(data, hash))
            return { ygg::View<ygg::Index<T>, BasicConstructorRepository>(*index, *this), false };

        const auto [index, success] = m_symbol_repository.template get_or_create_local_with_hash<T>(data, hash);
        return { ygg::View<ygg::Index<T>, BasicConstructorRepository>(index, *this), success };
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
    std::optional<ygg::View<ygg::Index<T>, BasicConstructorRepository>> find(const ygg::Data<T>& data) const noexcept
    {
        return find_with_hash<T>(data, FamilyConstructorSymbolRepository<Family>::template hash<T>(data));
    }

    template<typename T>
    std::pair<ygg::View<ygg::Index<T>, BasicConstructorRepository>, bool> get_or_create(ygg::Data<T>& data)
    {
        return get_or_create_with_hash<T>(data, FamilyConstructorSymbolRepository<Family>::template hash<T>(data));
    }

    template<typename T>
    const ygg::Data<T>& operator[](ygg::Index<T> index) const noexcept
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
    const BasicConstructorRepository& get_canonical_context(ygg::Index<T>) const noexcept
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

    ConstructorRepositoryPtrFor<Family> create(std::shared_ptr<const tyr::formalism::planning::Repository> planning_repository)
    {
        return ConstructorRepositoryPtrFor<Family>(new ConstructorRepositoryFor<Family>(m_next_index++, std::move(planning_repository)));
    }
};

template<FamilyTag Family>
using ConstructorRepositoryFactoryFor = BasicConstructorRepositoryFactory<Family>;

template<FamilyTag Family, typename Tag>
    requires FamilyConceptConstructorTag<Family, Tag>
using FamilyConceptView = ygg::View<ygg::Index<FamilyConcept<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family, typename Tag>
    requires FamilyRoleConstructorTag<Family, Tag>
using FamilyRoleView = ygg::View<ygg::Index<FamilyRole<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family, typename Tag>
    requires FamilyBooleanConstructorTag<Family, Tag>
using FamilyBooleanView = ygg::View<ygg::Index<FamilyBoolean<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family, typename Tag>
    requires FamilyNumericalConstructorTag<Family, Tag>
using FamilyNumericalView = ygg::View<ygg::Index<FamilyNumerical<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<FamilyTag Family, CategoryTag Category>
using FamilyConstructorView = ygg::View<ygg::Index<FamilyConstructor<Family, Category>>, ConstructorRepositoryFor<Family>>;

using BaseConstructorRepository = ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;
using BaseConstructorRepositoryPtr = ConstructorRepositoryPtrFor<runir::kr::BaseFamilyTag>;
using BaseConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>;

using ExtConstructorRepository = ConstructorRepositoryFor<runir::kr::ExtFamilyTag>;
using ExtConstructorRepositoryPtr = ConstructorRepositoryPtrFor<runir::kr::ExtFamilyTag>;
using ExtConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::ExtFamilyTag>;

using UnsConstructorRepository = ConstructorRepositoryFor<runir::kr::UnsFamilyTag>;
using UnsConstructorRepositoryPtr = ConstructorRepositoryPtrFor<runir::kr::UnsFamilyTag>;
using UnsConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::UnsFamilyTag>;

template<typename Tag>
    requires FamilyConceptConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseConceptView = FamilyConceptView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires FamilyRoleConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseRoleView = FamilyRoleView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires FamilyBooleanConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseBooleanView = FamilyBooleanView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires FamilyNumericalConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseNumericalView = FamilyNumericalView<runir::kr::BaseFamilyTag, Tag>;

template<CategoryTag Category>
using BaseConstructorView = FamilyConstructorView<runir::kr::BaseFamilyTag, Category>;

template<typename Tag>
    requires FamilyConceptConstructorTag<runir::kr::ExtFamilyTag, Tag>
using ExtConceptView = FamilyConceptView<runir::kr::ExtFamilyTag, Tag>;

template<typename Tag>
    requires FamilyRoleConstructorTag<runir::kr::ExtFamilyTag, Tag>
using ExtRoleView = FamilyRoleView<runir::kr::ExtFamilyTag, Tag>;

template<typename Tag>
    requires FamilyBooleanConstructorTag<runir::kr::ExtFamilyTag, Tag>
using ExtBooleanView = FamilyBooleanView<runir::kr::ExtFamilyTag, Tag>;

template<typename Tag>
    requires FamilyNumericalConstructorTag<runir::kr::ExtFamilyTag, Tag>
using ExtNumericalView = FamilyNumericalView<runir::kr::ExtFamilyTag, Tag>;

template<CategoryTag Category>
using ExtConstructorView = FamilyConstructorView<runir::kr::ExtFamilyTag, Category>;

template<typename Tag>
    requires FamilyConceptConstructorTag<runir::kr::UnsFamilyTag, Tag>
using UnsConceptView = FamilyConceptView<runir::kr::UnsFamilyTag, Tag>;

template<typename Tag>
    requires FamilyRoleConstructorTag<runir::kr::UnsFamilyTag, Tag>
using UnsRoleView = FamilyRoleView<runir::kr::UnsFamilyTag, Tag>;

template<typename Tag>
    requires FamilyBooleanConstructorTag<runir::kr::UnsFamilyTag, Tag>
using UnsBooleanView = FamilyBooleanView<runir::kr::UnsFamilyTag, Tag>;

template<typename Tag>
    requires FamilyNumericalConstructorTag<runir::kr::UnsFamilyTag, Tag>
using UnsNumericalView = FamilyNumericalView<runir::kr::UnsFamilyTag, Tag>;

template<CategoryTag Category>
using UnsConstructorView = FamilyConstructorView<runir::kr::UnsFamilyTag, Category>;

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
