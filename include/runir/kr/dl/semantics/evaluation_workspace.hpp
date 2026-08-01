#ifndef RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_
#define RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_

#include <yggdrasil/core/config.hpp>

#include <limits>
#include <vector>

namespace runir::kr::dl::semantics
{

class EvaluationWorkspace
{
private:
    std::vector<ygg::uint_t> m_distance_queue;
    std::vector<ygg::uint_t> m_distance_values;

public:
    EvaluationWorkspace() = default;

    void prepare_distance(ygg::uint_t num_objects)
    {
        m_distance_queue.clear();
        m_distance_queue.reserve(num_objects);
        m_distance_values.assign(num_objects, std::numeric_limits<ygg::uint_t>::max());
    }

    auto& get_distance_queue() noexcept { return m_distance_queue; }
    auto& get_distance_values() noexcept { return m_distance_values; }
};

}

#endif
