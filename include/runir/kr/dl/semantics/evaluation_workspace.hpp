#ifndef RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_
#define RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_

#include <tyr/formalism/declarations.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/database/semantics/distance.hpp>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics
{

class EvaluationWorkspace
{
private:
    std::vector<ygg::database::DistanceWorkspace<QueryValues>> m_distance_workspaces;  ///< One per vertex byte width.
    ygg::database::Workspace<QueryValues> m_database_workspace;

public:
    EvaluationWorkspace() = default;

    auto& get_distance_workspace(const ygg::database::DistancePlan<QueryValues>& plan)
    {
        for (auto& workspace : m_distance_workspaces)
            if (workspace.matches(plan))
                return workspace;
        return m_distance_workspaces.emplace_back(plan);
    }
    auto& get_database_workspace() noexcept { return m_database_workspace; }
};

}

#endif
