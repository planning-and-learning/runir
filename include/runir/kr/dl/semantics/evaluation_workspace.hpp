#ifndef RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_
#define RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_

#include <algorithm>
#include <deque>
#include <limits>
#include <optional>
#include <span>
#include "runir/kr/dl/semantics/distance/feature.hpp"
#include "runir/kr/dl/semantics/heuristics/projection.hpp"

#include <tyr/formalism/declarations.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/database/semantics/distance.hpp>
#include <yggdrasil/database/semantics/generic_join.hpp>
#include <yggdrasil/database/semantics/min_distance.hpp>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics
{

using FullDistanceFeature = DistanceFeature<ygg::database::MinDistance<QueryValues, heuristics::ProjectionHeuristic>, heuristics::ProjectionHeuristic>;

class EvaluationWorkspace
{
private:
    struct KeyedDistanceFeature
    {
        const void* formalism = nullptr;
        const void* constructors = nullptr;
        ygg::uint_t constructor = 0;
        FullDistanceFeature feature;
    };
    std::deque<KeyedDistanceFeature> m_distance_features;  ///< A deque keeps references stable.
    std::vector<ygg::database::GenericJoinWorkspace<QueryValues>> m_generic_join_workspaces;  ///< One per generic join plan.
    ygg::database::Workspace<QueryValues> m_database_workspace;

public:
    EvaluationWorkspace() = default;

    // ponytail: linear scan over distinct plans; key by query index if features carry many generic joins.
    auto& get_generic_join_workspace(const ygg::database::GenericJoinPlan<QueryValues>& plan)
    {
        for (auto& workspace : m_generic_join_workspaces)
            if (workspace.matches(plan))
                return workspace;
        return m_generic_join_workspaces.emplace_back(plan);
    }
    auto& get_database_workspace() noexcept { return m_database_workspace; }
    // ponytail: linear scan over distance features; key by constructor index if programs carry many.
    FullDistanceFeature& get_distance_feature(const void* formalism, const void* constructors, ygg::uint_t constructor)
    {
        for (auto& entry : m_distance_features)
            if (entry.formalism == formalism && entry.constructors == constructors && entry.constructor == constructor)
                return entry.feature;
        auto& entry = m_distance_features.emplace_back();
        entry.formalism = formalism;
        entry.constructors = constructors;
        entry.constructor = constructor;
        return entry.feature;
    }
};

}

#endif
