#ifndef RUNIR_SRC_GRAPHS_ALGORITHMS_NAUTY_IMPL_HPP_
#define RUNIR_SRC_GRAPHS_ALGORITHMS_NAUTY_IMPL_HPP_

#include <cstddef>
#include <nausparse.h>
#include <nauty.h>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::graphs::nauty::details
{

class SparseGraphImpl
{
private:
    std::size_t m_nde;
    urd::Store<urd::sequence<std::size_t>> m_offsets;
    int m_num_vertices;
    urd::Store<urd::sequence<int>> m_degrees;
    urd::Store<urd::sequence<int>> m_edges;
    urd::Store<urd::sequence<int>> m_labels;
    urd::Store<urd::sequence<int>> m_partitions;
    urd::Store<urd::sequence<int>> m_orbits;
    urd::Store<urd::sequence<ygg::uint_t>> m_coloring;
    bool m_is_canonical;
    urd::Store<urd::sequence<int>> m_permutation;
    urd::Store<urd::sequence<int>> m_inverse_permutation;

    sparsegraph m_graph;

    void initialize_sparse_graph();

public:
    SparseGraphImpl(std::size_t nde,
                    urd::Store<urd::sequence<std::size_t>> offsets,
                    int num_vertices,
                    urd::Store<urd::sequence<int>> degrees,
                    urd::Store<urd::sequence<int>> edges,
                    urd::Store<urd::sequence<int>> labels,
                    urd::Store<urd::sequence<int>> partitions,
                    urd::Store<urd::sequence<ygg::uint_t>> coloring);

    SparseGraphImpl(const SparseGraphImpl& other);
    auto operator=(const SparseGraphImpl& other) -> SparseGraphImpl&;
    SparseGraphImpl(SparseGraphImpl&& other) noexcept;
    auto operator=(SparseGraphImpl&& other) noexcept -> SparseGraphImpl&;
    ~SparseGraphImpl();

    void canonize();

    [[nodiscard]] auto get_num_directed_edges() const noexcept -> std::size_t;
    [[nodiscard]] auto get_num_vertices() const noexcept -> int;
    [[nodiscard]] auto get_offsets() const noexcept -> const urd::Store<urd::sequence<std::size_t>>&;
    [[nodiscard]] auto get_degrees() const noexcept -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_edges() const noexcept -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_labels() const noexcept -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_partitions() const noexcept -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_orbits() const noexcept -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_coloring() const noexcept -> const urd::Store<urd::sequence<ygg::uint_t>>&;
    [[nodiscard]] auto get_permutation() const -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_inverse_permutation() const -> const urd::Store<urd::sequence<int>>&;
};

}  // namespace runir::graphs::nauty::details

#endif
