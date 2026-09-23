#ifndef RUNIR_KR_PS_EXT_DETAIL_PREDECESSORS_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PREDECESSORS_HPP_

#include "runir/kr/ps/ext/detail/search_node.hpp"

#include <cassert>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::ps::ext::detail
{

enum class EdgeId : ygg::uint_t
{
};

/// Append-only explored transitions. IDs and interval boundaries remain stable across appends.
template<tyr::TaskKind Kind>
class Predecessors
{
private:
    std::vector<Predecessor<Kind>> m_edges;

public:
    EdgeId append(Predecessor<Kind> edge)
    {
        // Keep the one-past-end boundary representable as well as the edge ID.
        const auto size = ygg::to_uint_t(m_edges.size() + 1);
        m_edges.push_back(std::move(edge));
        return static_cast<EdgeId>(size - 1);
    }

    const Predecessor<Kind>& operator[](EdgeId id) const { return m_edges[static_cast<ygg::uint_t>(id)]; }

    EdgeId end_id() const noexcept { return static_cast<EdgeId>(m_edges.size()); }

    static EdgeId previous(EdgeId id) noexcept
    {
        assert(static_cast<ygg::uint_t>(id) != 0);
        return static_cast<EdgeId>(static_cast<ygg::uint_t>(id) - 1);
    }

    auto size() const noexcept { return m_edges.size(); }
    auto begin() const noexcept { return m_edges.begin(); }
    auto end() const noexcept { return m_edges.end(); }
};

}  // namespace runir::kr::ps::ext::detail

#endif
