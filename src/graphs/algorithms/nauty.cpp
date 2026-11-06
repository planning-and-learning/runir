#include "runir/graphs/algorithms/nauty.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

#include "nauty_impl.hpp"

#include <numeric>
#include <utility>

namespace runir::graphs::nauty
{

SparseGraph::SparseGraph(std::size_t nde,
                         urd::Store<urd::sequence<std::size_t>> offsets,
                         int num_vertices,
                         urd::Store<urd::sequence<int>> degrees,
                         urd::Store<urd::sequence<int>> edges,
                         urd::Store<urd::sequence<int>> labels,
                         urd::Store<urd::sequence<int>> partitions,
                         urd::Store<urd::sequence<ygg::uint_t>> coloring) :
    m_impl(std::make_unique<details::SparseGraphImpl>(nde,
                                                      std::move(offsets),
                                                      num_vertices,
                                                      std::move(degrees),
                                                      std::move(edges),
                                                      std::move(labels),
                                                      std::move(partitions),
                                                      std::move(coloring)))
{
}

SparseGraph::SparseGraph() : SparseGraph(0, {}, 0, {}, {}, {}, {}, {}) {}

SparseGraph::SparseGraph(const SparseGraph& other) : m_impl(std::make_unique<details::SparseGraphImpl>(*other.m_impl)) {}

auto SparseGraph::operator=(const SparseGraph& other) -> SparseGraph&
{
    if (this != &other)
        m_impl = std::make_unique<details::SparseGraphImpl>(*other.m_impl);
    return *this;
}

SparseGraph::SparseGraph(SparseGraph&& other) noexcept = default;
auto SparseGraph::operator=(SparseGraph&& other) noexcept -> SparseGraph& = default;
SparseGraph::~SparseGraph() = default;

auto SparseGraph::canonize() -> SparseGraph&
{
    m_impl->canonize();
    return *this;
}

auto SparseGraph::get_num_directed_edges() const -> std::size_t { return m_impl->get_num_directed_edges(); }
auto SparseGraph::get_num_vertices() const -> int { return m_impl->get_num_vertices(); }
auto SparseGraph::get_offsets() const -> const urd::Store<urd::sequence<std::size_t>>& { return m_impl->get_offsets(); }
auto SparseGraph::get_degrees() const -> const urd::Store<urd::sequence<int>>& { return m_impl->get_degrees(); }
auto SparseGraph::get_edges() const -> const urd::Store<urd::sequence<int>>& { return m_impl->get_edges(); }
auto SparseGraph::get_labels() const -> const urd::Store<urd::sequence<int>>& { return m_impl->get_labels(); }
auto SparseGraph::get_partitions() const -> const urd::Store<urd::sequence<int>>& { return m_impl->get_partitions(); }
auto SparseGraph::get_orbits() const -> const urd::Store<urd::sequence<int>>& { return m_impl->get_orbits(); }
auto SparseGraph::get_coloring() const -> const urd::Store<urd::sequence<ygg::uint_t>>& { return m_impl->get_coloring(); }
auto SparseGraph::get_permutation() const -> const urd::Store<urd::sequence<int>>& { return m_impl->get_permutation(); }
auto SparseGraph::get_inverse_permutation() const -> const urd::Store<urd::sequence<int>>& { return m_impl->get_inverse_permutation(); }

auto apply_permutation(const urd::Store<urd::sequence<int>>& permutation, urd::Store<urd::sequence<int>>& values) -> urd::Store<urd::sequence<int>>&
{
    for (auto& value : values)
        value = permutation.at(static_cast<std::size_t>(value));
    return values;
}

auto compute_label_permutation(const SparseGraph& source, const SparseGraph& target) -> urd::Store<urd::sequence<int>>
{
    if (!(source == target))
        throw std::runtime_error("runir::graphs::nauty::compute_label_permutation: graphs have different canonical structure.");

    auto target_label_to_index = urd::Store<urd::map<int, int>>();
    for (int i = 0; i < target.get_num_vertices(); ++i)
        target_label_to_index.emplace(target.get_labels().at(static_cast<std::size_t>(i)), i);

    auto result = urd::Store<urd::sequence<int>>(static_cast<std::size_t>(source.get_num_vertices()));
    for (int i = 0; i < source.get_num_vertices(); ++i)
        result[i] = target_label_to_index.at(source.get_labels().at(static_cast<std::size_t>(i)));
    return result;
}

auto compute_permutation(const SparseGraph& source, const SparseGraph& target) -> urd::Store<urd::sequence<int>>
{
    auto result = urd::Store<urd::sequence<int>>(static_cast<std::size_t>(source.get_num_vertices()));
    std::iota(result.begin(), result.end(), 0);

    apply_permutation(source.get_inverse_permutation(),
                      apply_permutation(compute_label_permutation(source, target), apply_permutation(source.get_permutation(), result)));
    return result;
}

}  // namespace runir::graphs::nauty
