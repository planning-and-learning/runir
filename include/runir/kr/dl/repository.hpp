#ifndef RUNIR_REPOSITORY_HPP_
#define RUNIR_REPOSITORY_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <type_traits>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
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
using FamilyReferenceTypes = std::conditional_t<
    std::same_as<Family, runir::kr::ExtFamilyTag>,
    ygg::TypeList<Argument<ConceptTag>, Argument<RoleTag>, Argument<BooleanTag>, Argument<NumericalTag>, Register<ConceptTag>, Register<RoleTag>>,
    ygg::TypeList<>>;

template<FamilyTag Family>
using FamilyConstructorRepositoryTypes = ygg::ConcatTypeListsT<FamilyConceptTypes<Family>,
                                                               FamilyRoleTypes<Family>,
                                                               FamilyBooleanTypes<Family>,
                                                               FamilyNumericalTypes<Family>,
                                                               FamilyConstructorTypes<Family>,
                                                               FamilyReferenceTypes<Family>>;

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
        if (auto index = m_symbol_repository.template find_local<T>(data))
            return ygg::View<ygg::Index<T>, BasicConstructorRepository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<ygg::View<ygg::Index<T>, BasicConstructorRepository>, bool> get_or_create(ygg::Data<T>& data)
    {
        const auto [index, created] = m_symbol_repository.template get_or_create_local<T>(data);
        return { ygg::View<ygg::Index<T>, BasicConstructorRepository>(index, *this), created };
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

#ifndef RUNIR_HEADER_INSTANTIATION
extern template class BasicConstructorRepositoryFactory<runir::kr::BaseFamilyTag>;
extern template class BasicConstructorRepositoryFactory<runir::kr::ExtFamilyTag>;
extern template class BasicConstructorRepositoryFactory<runir::kr::UnsFamilyTag>;
#endif

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
