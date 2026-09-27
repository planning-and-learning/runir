#ifndef RUNIR_REPOSITORY_HPP_
#define RUNIR_REPOSITORY_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/construction_metadata.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/query_construction.hpp"
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
#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl
{

template<FamilyTag Family>
using FamilyConceptTypes = ygg::MapTypeListSecondT<Concept, Family, FamilyConceptConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyRoleTypes = ygg::MapTypeListSecondT<Role, Family, FamilyRoleConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyBooleanTypes = ygg::MapTypeListSecondT<Boolean, Family, FamilyBooleanConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyNumericalTypes = ygg::MapTypeListSecondT<Numerical, Family, FamilyNumericalConstructorTags<Family>>;

template<FamilyTag Family>
using FamilyConstructorTypes = ygg::MapTypeListSecondT<Constructor, Family, CategoryTags>;

template<FamilyTag Family>
using FamilyQueryTypes = ygg::MapTypeListSecondT<Query, Family, QueryConstructorTags>;

template<FamilyTag Family>
using FamilyReferenceTypes = std::conditional_t<
    std::same_as<Family, runir::kr::ExtFamilyTag>,
    ygg::TypeList<Argument<ConceptTag>, Argument<RoleTag>, Argument<BooleanTag>, Argument<NumericalTag>, Register<ConceptTag>, Register<RoleTag>>,
    ygg::TypeList<>>;

template<FamilyTag Family>
using FamilyConstructorRepositoryTypes =
    ygg::ConcatTypeListsT<FamilyConceptTypes<Family>,
                          FamilyRoleTypes<Family>,
                          FamilyBooleanTypes<Family>,
                          FamilyNumericalTypes<Family>,
                          FamilyConstructorTypes<Family>,
                          FamilyReferenceTypes<Family>,
                          FamilyQueryTypes<Family>,
                          ygg::TypeList<Query<Family>, QueryColumn, QueryProjection<Family, ConceptTag>, QueryProjection<Family, RoleTag>>>;

template<FamilyTag Family>
using FamilyConstructorSymbolRepository = ygg::ApplyTypeListT<ygg::formalism::SymbolRepository, FamilyConstructorRepositoryTypes<Family>>;

template<FamilyTag Family>
using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, FamilyConstructorRepositoryTypes<Family>>;

using BaseBuilder = Builder<runir::kr::BaseFamilyTag>;
using ExtBuilder = Builder<runir::kr::ExtFamilyTag>;
using UnsBuilder = Builder<runir::kr::UnsFamilyTag>;

template<typename T, typename B>
    requires(std::same_as<B, BaseBuilder> || std::same_as<B, ExtBuilder> || std::same_as<B, UnsBuilder>)
[[nodiscard]] auto checkout(B& builder)
{
    auto data = builder.template get_builder<T>();
    data->clear();
    return data;
}

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

    /// Raw symbol interning. Use the free get_or_create() to derive schemas and staticness.
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

template<FamilyTag Family, typename T>
[[nodiscard]] auto get_or_create(BasicConstructorRepository<Family>& repository, ygg::Data<T>& data)
{
    canonicalize(data);
    detail::prepare(data, repository);
    return repository.get_or_create(data);
}

}

#endif
