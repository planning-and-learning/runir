#include "runir/kr/dl/semantics/denotation_caches.hpp"

#include <array>
#include <tuple>
#include <tyr/datalog/lifted/database/semantics/join_index.hpp>
#include <tyr/datalog/lifted/database/semantics/relation_view.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>

namespace runir::kr::dl::semantics
{

struct DenotationCaches::Impl
{
    template<CategoryTag Category>
    using Cache = urd::Store<urd::map<ConstructorView<Category>, DenotationView<Category>>>;

    using QueryCache = urd::Store<urd::map<QueryView<>, QueryDenotationView>>;

    struct Partition
    {
        std::tuple<Cache<ConceptTag>, Cache<RoleTag>, Cache<BooleanTag>, Cache<NumericalTag>> values;
        QueryCache queries;

        void clear() noexcept
        {
            std::apply([](auto&... caches) { (caches.clear(), ...); }, values);
            queries.clear();
        }
    };

    template<CategoryTag Category>
    Cache<Category>& get(bool is_static) noexcept
    { return std::get<Cache<Category>>(partitions[is_static].values); }

    std::array<Partition, 2> partitions;
    tyr::datalog::database::JoinIndexCache static_join_indexes;
};

DenotationCaches::DenotationCaches() : m_impl(std::make_unique<Impl>()) {}
DenotationCaches::DenotationCaches(DenotationCaches&&) noexcept = default;
DenotationCaches& DenotationCaches::operator=(DenotationCaches&&) noexcept = default;
DenotationCaches::~DenotationCaches() = default;

template<CategoryTag Category>
std::optional<DenotationView<Category>> DenotationCaches::find(bool is_static, ConstructorView<Category> constructor) const
{
    const auto& cache = m_impl->template get<Category>(is_static);
    if (const auto it = cache.find(constructor); it != cache.end())
        return it->second;
    return std::nullopt;
}

template<CategoryTag Category>
DenotationView<Category> DenotationCaches::insert(bool is_static, ConstructorView<Category> constructor, DenotationView<Category> denotation)
{ return m_impl->template get<Category>(is_static).emplace(constructor, denotation).first->second; }

template<CategoryTag Category>
void DenotationCaches::erase(bool is_static, ConstructorView<Category> constructor)
{ m_impl->template get<Category>(is_static).erase(constructor); }

template<CategoryTag Category>
std::size_t DenotationCaches::size(bool is_static) const
{ return m_impl->template get<Category>(is_static).size(); }

std::optional<QueryDenotationView> DenotationCaches::find_query(bool is_static, QueryView<> query) const
{
    const auto& cache = m_impl->partitions[is_static].queries;
    if (const auto it = cache.find(query); it != cache.end())
        return it->second;
    return std::nullopt;
}

QueryDenotationView DenotationCaches::insert_query(bool is_static, QueryView<> query, QueryDenotationView result)
{ return m_impl->partitions[is_static].queries.emplace(query, result).first->second; }

std::vector<QueryDenotationView> DenotationCaches::get_queries(bool is_static) const
{
    auto results = std::vector<QueryDenotationView>();
    for (const auto& [query, result] : m_impl->partitions[is_static].queries)
        results.push_back(result);
    return results;
}

tyr::datalog::database::JoinIndexCache& DenotationCaches::get_static_join_indexes() noexcept { return m_impl->static_join_indexes; }
const tyr::datalog::database::JoinIndexCache& DenotationCaches::get_static_join_indexes() const noexcept { return m_impl->static_join_indexes; }

void DenotationCaches::reset_dynamic() noexcept { m_impl->partitions[false].clear(); }

void DenotationCaches::reset_all() noexcept
{
    m_impl->static_join_indexes.clear();
    reset_dynamic();
    m_impl->partitions[true].clear();
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage): explicit instantiations, declarations, which no function gives
#define RUNIR_DENOTATION_CACHES(Category)                                                                                            \
    template std::optional<DenotationView<Category>> DenotationCaches::find<Category>(bool, ConstructorView<Category>) const;        \
    template DenotationView<Category> DenotationCaches::insert<Category>(bool, ConstructorView<Category>, DenotationView<Category>); \
    template void DenotationCaches::erase<Category>(bool, ConstructorView<Category>);                                                \
    template std::size_t DenotationCaches::size<Category>(bool) const;
// NOLINTEND(cppcoreguidelines-macro-usage)

RUNIR_DENOTATION_CACHES(ConceptTag)
RUNIR_DENOTATION_CACHES(RoleTag)
RUNIR_DENOTATION_CACHES(BooleanTag)
RUNIR_DENOTATION_CACHES(NumericalTag)

}  // namespace runir::kr::dl::semantics
