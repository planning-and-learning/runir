#ifndef RUNIR_KR_DL_SEMANTICS_DENOTATION_CACHES_HPP_
#define RUNIR_KR_DL_SEMANTICS_DENOTATION_CACHES_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <array>
#include <cstdint>
#include <tuple>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/database/semantics/join_index.hpp>
#include <yggdrasil/database/semantics/relation_repository.hpp>

namespace runir::kr::dl::semantics
{

/// Memoization for a fixed task and constructor repository factory. Result
/// repositories own every stored view and must outlive these entries. Reset
/// dynamic entries before changing state, registers, or arguments. Reset all
/// entries before releasing static rows or changing the repository factory.
class DenotationCaches
{
    template<CategoryTag Category>
    using Cache = ygg::UnorderedMap<ConstructorView<Category>, DenotationView<Category>>;

    using QueryCache = ygg::UnorderedMap<QueryView<>, QueryDenotationView>;
    /// Per program, the generation of its feature's evaluation in this state (DatalogFeature).
    using ProgramCache = ygg::UnorderedMap<DatalogProgramView, uint64_t>;

    struct Partition
    {
        std::tuple<Cache<ConceptTag>, Cache<RoleTag>, Cache<BooleanTag>, Cache<NumericalTag>> values;
        QueryCache queries;
        ProgramCache programs;

        void clear() noexcept
        {
            std::apply([](auto&... caches) { (caches.clear(), ...); }, values);
            queries.clear();
            programs.clear();
        }
    };

    std::array<Partition, 2> m_partitions;
    ygg::database::JoinIndexCache m_static_join_indexes;

public:
    DenotationCaches() = default;
    DenotationCaches(const DenotationCaches&) = delete;
    DenotationCaches& operator=(const DenotationCaches&) = delete;
    DenotationCaches(DenotationCaches&&) = default;
    DenotationCaches& operator=(DenotationCaches&&) = default;
    ~DenotationCaches() = default;

    template<CategoryTag Category>
    auto& get(bool is_static) noexcept
    { return std::get<Cache<Category>>(m_partitions[is_static].values); }

    template<CategoryTag Category>
    [[nodiscard]] const auto& get(bool is_static) const noexcept
    { return std::get<Cache<Category>>(m_partitions[is_static].values); }

    auto& get_queries(bool is_static) noexcept { return m_partitions[is_static].queries; }
    [[nodiscard]] const auto& get_queries(bool is_static) const noexcept { return m_partitions[is_static].queries; }
    [[nodiscard]] auto& get_programs(bool is_static) noexcept { return m_partitions[is_static].programs; }
    auto& get_static_join_indexes() noexcept { return m_static_join_indexes; }
    [[nodiscard]] const auto& get_static_join_indexes() const noexcept { return m_static_join_indexes; }

    void reset_dynamic() noexcept { m_partitions[false].clear(); }

    void reset_all() noexcept
    {
        m_static_join_indexes.clear();
        reset_dynamic();
        m_partitions[true].clear();
    }
};

}  // namespace runir::kr::dl::semantics

#endif
