#ifndef RUNIR_SRC_GRAPHS_ALGORITHMS_NAUTY_IMPL_HPP_
#define RUNIR_SRC_GRAPHS_ALGORITHMS_NAUTY_IMPL_HPP_

#include "runir/common/config.hpp"

#include <cstddef>
#include <vector>

#ifdef RUNIR_HAS_NAUTY
#include <nausparse.h>
#include <nauty.h>
#endif

namespace runir::graphs::nauty::details
{

class SparseGraphImpl
{
private:
    std::size_t m_nde;
    std::vector<std::size_t> m_offsets;
    int m_num_vertices;
    std::vector<int> m_degrees;
    std::vector<int> m_edges;
    std::vector<int> m_labels;
    std::vector<int> m_partitions;
    std::vector<int> m_orbits;
    std::vector<uint_t> m_coloring;
    bool m_is_canonical;
    std::vector<int> m_permutation;
    std::vector<int> m_inverse_permutation;

#ifdef RUNIR_HAS_NAUTY
    sparsegraph m_graph;

    void initialize_sparse_graph();
#endif

public:
    SparseGraphImpl(std::size_t nde,
                    std::vector<std::size_t> offsets,
                    int num_vertices,
                    std::vector<int> degrees,
                    std::vector<int> edges,
                    std::vector<int> labels,
                    std::vector<int> partitions,
                    std::vector<uint_t> coloring);

    SparseGraphImpl(const SparseGraphImpl& other);
    auto operator=(const SparseGraphImpl& other) -> SparseGraphImpl&;
    SparseGraphImpl(SparseGraphImpl&& other) noexcept;
    auto operator=(SparseGraphImpl&& other) noexcept -> SparseGraphImpl&;
    ~SparseGraphImpl();

    void canonize();

    auto get_num_directed_edges() const noexcept -> std::size_t;
    auto get_num_vertices() const noexcept -> int;
    auto get_offsets() const noexcept -> const std::vector<std::size_t>&;
    auto get_degrees() const noexcept -> const std::vector<int>&;
    auto get_edges() const noexcept -> const std::vector<int>&;
    auto get_labels() const noexcept -> const std::vector<int>&;
    auto get_partitions() const noexcept -> const std::vector<int>&;
    auto get_orbits() const noexcept -> const std::vector<int>&;
    auto get_coloring() const noexcept -> const std::vector<uint_t>&;
    auto get_permutation() const -> const std::vector<int>&;
    auto get_inverse_permutation() const -> const std::vector<int>&;
};

}  // namespace runir::graphs::nauty::details

#endif
