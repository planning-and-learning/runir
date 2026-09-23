#ifndef RUNIR_KR_DL_SEMANTICS_DENOTATION_CACHES_HPP_
#define RUNIR_KR_DL_SEMANTICS_DENOTATION_CACHES_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <array>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/containers/unique_object_pool.hpp>
#include <yggdrasil/database/relation.hpp>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::kr::dl::semantics
{

/// Denotations for one fixed task and constructor repository factory. Static entries
/// survive changes to the evaluation context; clear(false) before changing state,
/// registers, or arguments. Each partition owns its denotation storage and reuses
/// its capacity after clearing. All returned denotations and query results borrow
/// their partition and remain valid until it is cleared.
/// Constructor repositories and relation workspaces must outlive their cached views.
/// Clearing static entries also clears dynamic entries, which may borrow static rows.
template<FamilyTag Family>
struct DenotationCaches
{
    template<CategoryTag Category>
    using Cache = ygg::UnorderedMap<FamilyConstructorView<Family, Category>, DenotationView<Category>>;

    using QueryCache = ygg::UnorderedMap<FamilyQueryView<Family>, ygg::database::RelationView<>>;

private:
    struct Partition
    {
        DenotationRepositoryPtr denotations;
        std::tuple<Cache<ConceptTag>, Cache<RoleTag>, Cache<BooleanTag>, Cache<NumericalTag>> values;
        QueryCache queries;
        std::vector<ygg::UniqueObjectPoolPtr<ygg::database::Relation<>>> relations;

        explicit Partition(const DenotationRepository& persistent) :
            denotations(persistent.get_factory().create_shared(persistent.get_formalism_repository_ptr()))
        {
        }

        void clear() noexcept
        {
            std::apply([](auto&... caches) { (caches.clear(), ...); }, values);
            queries.clear();
            relations.clear();
            denotations->clear();
        }
    };

    std::array<Partition, 2> m_partitions;

public:
    explicit DenotationCaches(const DenotationRepository& persistent) : m_partitions { Partition(persistent), Partition(persistent) } {}
    DenotationCaches(const DenotationCaches&) = delete;
    DenotationCaches& operator=(const DenotationCaches&) = delete;
    DenotationCaches(DenotationCaches&&) = default;
    DenotationCaches& operator=(DenotationCaches&&) = default;

    auto& get_repository(bool is_static) noexcept { return *m_partitions[is_static].denotations; }
    const auto& get_repository(bool is_static) const noexcept { return *m_partitions[is_static].denotations; }

    template<CategoryTag Category>
    auto& get(bool is_static) noexcept
    {
        return std::get<Cache<Category>>(m_partitions[is_static].values);
    }

    template<CategoryTag Category>
    const auto& get(bool is_static) const noexcept
    {
        return std::get<Cache<Category>>(m_partitions[is_static].values);
    }

    auto& get_queries(bool is_static) noexcept { return m_partitions[is_static].queries; }
    const auto& get_queries(bool is_static) const noexcept { return m_partitions[is_static].queries; }

    auto retain(bool is_static, ygg::UniqueObjectPoolPtr<ygg::database::Relation<>> relation)
    {
        const auto view = relation->view();
        m_partitions[is_static].relations.push_back(std::move(relation));
        return view;
    }

    auto retain(bool, ygg::database::RelationView<> relation) noexcept { return relation; }

    void clear(bool is_static) noexcept
    {
        if (is_static)
            m_partitions[false].clear();
        m_partitions[is_static].clear();
    }

    void clear() noexcept { clear(true); }
};

}  // namespace runir::kr::dl::semantics

#endif
