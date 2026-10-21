#ifndef RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_
#define RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_

#include <algorithm>
#include <concepts>
#include <deque>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/distance/feature.hpp"
#include "runir/kr/dl/semantics/heuristics/projection.hpp"

#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/database/semantics/distance.hpp>
#include <yggdrasil/database/semantics/generic_join.hpp>
#include <yggdrasil/database/semantics/min_distance.hpp>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics
{

/// One n_distance in full evaluation: its edges and its heuristic, built at first use.
struct FullDistanceFeature
{
    DistanceFeature<ygg::database::MinDistance<heuristics::ProjectionHeuristic>> edges;
    heuristics::ProjectionHeuristic heuristic;
};

class EvaluationWorkspace
{
private:
    /// A feature built for one constructor of a constructor repository over a formalism repository.
    template<typename Feature>
    struct KeyedFeature
    {
        std::weak_ptr<const tyr::formalism::planning::Repository> formalism;  ///< Its control block identifies it while kept.
        const void* constructors = nullptr;
        size_t constructors_index = 0;
        ygg::uint_t constructor = 0;
        Feature feature;
    };
    std::deque<KeyedFeature<FullDistanceFeature>> m_distance_features;  ///< A deque keeps references stable.
    std::deque<KeyedFeature<MustPassFeature>> m_must_pass_features;
    std::vector<ygg::database::GenericJoinWorkspace> m_generic_join_workspaces;  ///< One per generic join plan.
    ygg::database::Workspace m_database_workspace;

    // ponytail: linear scan, and entries of dead repositories are kept; a constructor repository freed and replaced at
    // the same address with the same factory index over the same formalism would alias. Key by a repository-owned
    // cache if that happens.
    template<typename Feature, FamilyTag Family, std::invocable Make>
        requires std::same_as<std::invoke_result_t<Make&>, Feature>
    static Feature& find_or_make(std::deque<KeyedFeature<Feature>>& features,
                                 const std::shared_ptr<tyr::formalism::planning::Repository>& formalism,
                                 const ConstructorRepositoryFor<Family>& constructors,
                                 ygg::uint_t constructor,
                                 Make&& make)
    {
        const auto same_formalism = [&](const KeyedFeature<Feature>& entry)
        { return !entry.formalism.owner_before(formalism) && !formalism.owner_before(entry.formalism); };
        for (auto& entry : features)
            if (entry.constructors == &constructors && entry.constructors_index == constructors.get_index() && entry.constructor == constructor
                && same_formalism(entry))
                return entry.feature;
        return features.emplace_back(KeyedFeature<Feature> { formalism, &constructors, constructors.get_index(), constructor, make() }).feature;
    }

public:
    EvaluationWorkspace() = default;

    // ponytail: linear scan over distinct plans; key by query index if features carry many generic joins.
    auto& get_generic_join_workspace(const ygg::database::GenericJoinPlan& plan)
    {
        for (auto& workspace : m_generic_join_workspaces)
            if (workspace.matches(plan))
                return workspace;
        return m_generic_join_workspaces.emplace_back(plan);
    }
    auto& get_database_workspace() noexcept { return m_database_workspace; }
    /// The feature of a distance constructor of a constructor repository over a formalism repository, made by
    /// make() at first use. References stay valid while this workspace lives.
    template<FamilyTag Family, std::invocable Make>
        requires std::same_as<std::invoke_result_t<Make&>, FullDistanceFeature>
    FullDistanceFeature& get_distance_feature(const std::shared_ptr<tyr::formalism::planning::Repository>& formalism,
                                              const ConstructorRepositoryFor<Family>& constructors,
                                              ygg::uint_t constructor,
                                              Make&& make)
    {
        return find_or_make<FullDistanceFeature, Family>(m_distance_features, formalism, constructors, constructor, std::forward<Make>(make));
    }
    /// The feature of a must-pass query, like get_distance_feature().
    template<FamilyTag Family, std::invocable Make>
        requires std::same_as<std::invoke_result_t<Make&>, MustPassFeature>
    MustPassFeature& get_must_pass_feature(const std::shared_ptr<tyr::formalism::planning::Repository>& formalism,
                                           const ConstructorRepositoryFor<Family>& constructors,
                                           ygg::uint_t query,
                                           Make&& make)
    {
        return find_or_make<MustPassFeature, Family>(m_must_pass_features, formalism, constructors, query, std::forward<Make>(make));
    }
};

}

#endif
