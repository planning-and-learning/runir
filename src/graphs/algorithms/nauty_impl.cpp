#include "nauty_impl.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace runir::graphs::nauty::details
{

void SparseGraphImpl::initialize_sparse_graph()
{
    SG_INIT(m_graph);
    m_graph.nde = m_nde;
    m_graph.v = m_offsets.data();
    m_graph.nv = m_num_vertices;
    m_graph.d = m_degrees.data();
    m_graph.e = m_edges.data();
    m_graph.vlen = m_offsets.size();
    m_graph.dlen = m_degrees.size();
    m_graph.elen = m_edges.size();
}

SparseGraphImpl::SparseGraphImpl(std::size_t nde,
                                 std::vector<std::size_t> offsets,
                                 int num_vertices,
                                 std::vector<int> degrees,
                                 std::vector<int> edges,
                                 std::vector<int> labels,
                                 std::vector<int> partitions,
                                 std::vector<uint_t> coloring) :
    m_nde(nde),
    m_offsets(std::move(offsets)),
    m_num_vertices(num_vertices),
    m_degrees(std::move(degrees)),
    m_edges(std::move(edges)),
    m_labels(std::move(labels)),
    m_partitions(std::move(partitions)),
    m_orbits(static_cast<std::size_t>(m_num_vertices)),
    m_coloring(std::move(coloring)),
    m_is_canonical(false),
    m_graph()
{
    initialize_sparse_graph();
}

SparseGraphImpl::SparseGraphImpl(const SparseGraphImpl& other) :
    m_nde(other.m_nde),
    m_offsets(other.m_offsets),
    m_num_vertices(other.m_num_vertices),
    m_degrees(other.m_degrees),
    m_edges(other.m_edges),
    m_labels(other.m_labels),
    m_partitions(other.m_partitions),
    m_orbits(other.m_orbits),
    m_coloring(other.m_coloring),
    m_is_canonical(other.m_is_canonical),
    m_permutation(other.m_permutation),
    m_inverse_permutation(other.m_inverse_permutation),
    m_graph()
{
    initialize_sparse_graph();
}

auto SparseGraphImpl::operator=(const SparseGraphImpl& other) -> SparseGraphImpl&
{
    if (this == &other)
        return *this;

    m_nde = other.m_nde;
    m_offsets = other.m_offsets;
    m_num_vertices = other.m_num_vertices;
    m_degrees = other.m_degrees;
    m_edges = other.m_edges;
    m_labels = other.m_labels;
    m_partitions = other.m_partitions;
    m_orbits = other.m_orbits;
    m_coloring = other.m_coloring;
    m_is_canonical = other.m_is_canonical;
    m_permutation = other.m_permutation;
    m_inverse_permutation = other.m_inverse_permutation;

    initialize_sparse_graph();

    return *this;
}

SparseGraphImpl::SparseGraphImpl(SparseGraphImpl&& other) noexcept :
    m_nde(other.m_nde),
    m_offsets(std::move(other.m_offsets)),
    m_num_vertices(other.m_num_vertices),
    m_degrees(std::move(other.m_degrees)),
    m_edges(std::move(other.m_edges)),
    m_labels(std::move(other.m_labels)),
    m_partitions(std::move(other.m_partitions)),
    m_orbits(std::move(other.m_orbits)),
    m_coloring(std::move(other.m_coloring)),
    m_is_canonical(other.m_is_canonical),
    m_permutation(std::move(other.m_permutation)),
    m_inverse_permutation(std::move(other.m_inverse_permutation)),
    m_graph()
{
    initialize_sparse_graph();
}

auto SparseGraphImpl::operator=(SparseGraphImpl&& other) noexcept -> SparseGraphImpl&
{
    if (this == &other)
        return *this;

    m_nde = other.m_nde;
    m_offsets = std::move(other.m_offsets);
    m_num_vertices = other.m_num_vertices;
    m_degrees = std::move(other.m_degrees);
    m_edges = std::move(other.m_edges);
    m_labels = std::move(other.m_labels);
    m_partitions = std::move(other.m_partitions);
    m_orbits = std::move(other.m_orbits);
    m_coloring = std::move(other.m_coloring);
    m_is_canonical = other.m_is_canonical;
    m_permutation = std::move(other.m_permutation);
    m_inverse_permutation = std::move(other.m_inverse_permutation);

    initialize_sparse_graph();

    return *this;
}
SparseGraphImpl::~SparseGraphImpl() = default;

void SparseGraphImpl::canonize()
{
    if (m_is_canonical)
        return;

    static DEFAULTOPTIONS_SPARSEGRAPH(options);
    options.defaultptn = FALSE;
    options.getcanon = TRUE;
    options.digraph = FALSE;
    options.writeautoms = FALSE;

    auto stats = statsblk();
    auto canonical = SparseGraphImpl(*this);
    auto canonical_graph = sparsegraph {};
    SG_INIT(canonical_graph);

    sparsenauty(&m_graph, canonical.m_labels.data(), canonical.m_partitions.data(), canonical.m_orbits.data(), &options, &stats, &canonical_graph);
    sortlists_sg(&canonical_graph);

    canonical.m_nde = canonical_graph.nde;
    canonical.m_num_vertices = canonical_graph.nv;
    canonical.m_offsets.assign(canonical_graph.v, canonical_graph.v + canonical_graph.nv);
    canonical.m_degrees.assign(canonical_graph.d, canonical_graph.d + canonical_graph.nv);
    canonical.m_edges.assign(canonical_graph.e, canonical_graph.e + canonical_graph.nde);
    SG_FREE(canonical_graph);
    canonical.initialize_sparse_graph();

    auto label_to_index = std::vector<int>(static_cast<std::size_t>(m_num_vertices));
    auto canonical_label_to_index = std::vector<int>(static_cast<std::size_t>(m_num_vertices));
    for (int i = 0; i < m_num_vertices; ++i)
    {
        label_to_index[m_labels[i]] = i;
        canonical_label_to_index[canonical.m_labels[i]] = i;
    }

    canonical.m_permutation.resize(static_cast<std::size_t>(m_num_vertices));
    canonical.m_inverse_permutation.resize(static_cast<std::size_t>(m_num_vertices));
    for (int i = 0; i < m_num_vertices; ++i)
    {
        canonical.m_permutation[i] = label_to_index[canonical.m_labels[i]];
        canonical.m_inverse_permutation[i] = canonical_label_to_index[m_labels[i]];
    }

    canonical.m_is_canonical = true;
    *this = std::move(canonical);
}

auto SparseGraphImpl::get_num_directed_edges() const noexcept -> std::size_t { return m_nde; }
auto SparseGraphImpl::get_num_vertices() const noexcept -> int { return m_num_vertices; }
auto SparseGraphImpl::get_offsets() const noexcept -> const std::vector<std::size_t>& { return m_offsets; }
auto SparseGraphImpl::get_degrees() const noexcept -> const std::vector<int>& { return m_degrees; }
auto SparseGraphImpl::get_edges() const noexcept -> const std::vector<int>& { return m_edges; }
auto SparseGraphImpl::get_labels() const noexcept -> const std::vector<int>& { return m_labels; }
auto SparseGraphImpl::get_partitions() const noexcept -> const std::vector<int>& { return m_partitions; }
auto SparseGraphImpl::get_orbits() const noexcept -> const std::vector<int>& { return m_orbits; }
auto SparseGraphImpl::get_coloring() const noexcept -> const std::vector<uint_t>& { return m_coloring; }

auto SparseGraphImpl::get_permutation() const -> const std::vector<int>&
{
    if (!m_is_canonical)
        throw std::runtime_error("runir::graphs::nauty::SparseGraphImpl::get_permutation: graph is not canonical.");
    return m_permutation;
}

auto SparseGraphImpl::get_inverse_permutation() const -> const std::vector<int>&
{
    if (!m_is_canonical)
        throw std::runtime_error("runir::graphs::nauty::SparseGraphImpl::get_inverse_permutation: graph is not canonical.");
    return m_inverse_permutation;
}

}  // namespace runir::graphs::nauty::details
