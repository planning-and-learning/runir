#ifndef RUNIR_KR_DL_SEMANTICS_DENOTATION_CACHES_HPP_
#define RUNIR_KR_DL_SEMANTICS_DENOTATION_CACHES_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace runir::kr::dl::semantics
{

/// Memoization for a fixed task and constructor repository factory. Result
/// repositories own every stored view and must outlive these entries. Reset
/// dynamic entries before changing state, registers, or arguments. Reset all
/// entries before releasing static rows or changing the repository factory.
///
/// The maps sit behind a pointer: src/kr/dl/semantics/denotation_caches.cpp defines them and the members, for each
/// category, so that no other source completes a map (brokkr's template rules, rule 6).
class DenotationCaches
{
public:
    /// The maps: defined in denotation_caches.cpp.
    struct Impl;

    DenotationCaches();
    DenotationCaches(const DenotationCaches&) = delete;
    DenotationCaches& operator=(const DenotationCaches&) = delete;
    DenotationCaches(DenotationCaches&&) noexcept;
    DenotationCaches& operator=(DenotationCaches&&) noexcept;
    ~DenotationCaches();

    /// The cached denotation of the constructor in the static or the dynamic partition, if any.
    template<CategoryTag Category>
    [[nodiscard]] std::optional<DenotationView<Category>> find(bool is_static, ConstructorView<Category> constructor) const;
    /// Caches the denotation of the constructor; an entry already there is kept and returned.
    template<CategoryTag Category>
    DenotationView<Category> insert(bool is_static, ConstructorView<Category> constructor, DenotationView<Category> denotation);
    template<CategoryTag Category>
    void erase(bool is_static, ConstructorView<Category> constructor);
    /// The number of cached denotations of the category in a partition.
    template<CategoryTag Category>
    [[nodiscard]] std::size_t size(bool is_static) const;

    [[nodiscard]] std::optional<QueryDenotationView> find_query(bool is_static, QueryView<> query) const;
    /// Caches the result of the query; an entry already there is kept and returned.
    QueryDenotationView insert_query(bool is_static, QueryView<> query, QueryDenotationView result);
    /// The cached query results of a partition.
    [[nodiscard]] std::vector<QueryDenotationView> get_queries(bool is_static) const;

    void reset_dynamic() noexcept;
    void reset_all() noexcept;

private:
    std::unique_ptr<Impl> m_impl;
};

}  // namespace runir::kr::dl::semantics

#endif
