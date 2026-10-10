#ifndef RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_
#define RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_

#include <limits>
#include <tyr/formalism/declarations.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics
{

class EvaluationWorkspace
{
private:
    std::vector<ygg::uint_t> m_distance_queue;
    std::vector<ygg::uint_t> m_distance_values;
    ygg::database::Workspace<ObjectValues> m_database_workspace;

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
    auto& get_database_workspace() noexcept { return m_database_workspace; }
};

}

#endif
