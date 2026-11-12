#ifndef RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_
#define RUNIR_SEMANTICS_EVALUATION_WORKSPACE_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/datalog_feature.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <tyr/datalog/lifted/database/semantics/generic_join.hpp>
#include <tyr/datalog/lifted/database/semantics/operations.hpp>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <urd/repr/maps/postings.hpp>
#include <urd/repr/sequences/segmented.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::dl::semantics
{

class EvaluationWorkspace
{
private:
    /// A feature built for one constructor of a constructor repository over a formalism repository.
    template<typename Feature>
    struct KeyedFeature
    {
        std::weak_ptr<const tyr::formalism::planning::Repository> formalism;  ///< Its control block identifies it while kept.
        Feature feature;
    };
    /// The features of a constructor, by its repository's address and layer and its index, as positions in their sequence.
    using FeaturePositions = urd::Store<urd::map<std::tuple<std::uintptr_t, size_t, ygg::uint_t>, size_t>, urd::multi>;
    /// The layer the features' input snapshots come from (ygg::FreeRelationRepository::get_relation_builder); declared
    /// first, so it outlives them, and behind a pointer, so it stays where it is when the workspace moves.
    std::unique_ptr<ygg::FreeRelationRepository> m_layer = std::make_unique<ygg::FreeRelationRepository>();
    urd::Store<urd::sequence<KeyedFeature<DatalogFeature>>, urd::stable_refs> m_datalog_features;  ///< Stable references: callers keep the features they get.
    FeaturePositions m_datalog_feature_positions;
    urd::Store<urd::sequence<tyr::datalog::database::GenericJoinWorkspace>> m_generic_join_workspaces;  ///< One per generic join plan.
    tyr::datalog::database::Workspace m_database_workspace;
    /// A row's values, as evaluation encodes them (encode_object_row).
    urd::Store<urd::sequence<ygg::Value>> m_values;

    // ponytail: entries of dead repositories are kept; a constructor repository freed and replaced at the same address
    // with the same factory index over the same formalism would alias. Key by a repository-owned cache if that happens.
    // The constructor repository is a template parameter so that the body is checked where its type is complete.
    template<typename Feature, typename Constructors, std::invocable Make>
        requires std::same_as<std::invoke_result_t<Make&>, Feature>
    static Feature& find_or_make(urd::Store<urd::sequence<KeyedFeature<Feature>>, urd::stable_refs>& features,
                                 FeaturePositions& positions,
                                 const std::shared_ptr<tyr::formalism::planning::Repository>& formalism,
                                 const Constructors& constructors,
                                 ygg::uint_t constructor,
                                 Make make)
    {
        const auto same_formalism = [&](const KeyedFeature<Feature>& entry)
        { return !entry.formalism.owner_before(formalism) && !formalism.owner_before(entry.formalism); };
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): the address is an identity in the key, never turned back into a pointer
        const auto key = std::tuple { reinterpret_cast<std::uintptr_t>(&constructors), constructors.layer(), constructor };
        for (const auto [first, last] = positions.equal_range(key); const auto position : std::ranges::subrange(first, last) | std::views::values)
            if (same_formalism(features[position]))
                return features[position].feature;
        auto& feature = features.emplace_back(KeyedFeature<Feature> { formalism, make() }).feature;
        positions.insert({ key, features.size() - 1 });
        return feature;
    }

public:
    EvaluationWorkspace() = default;

    // ponytail: linear scan over distinct plans; key by query index if features carry many generic joins.
    auto& get_generic_join_workspace(const tyr::datalog::database::GenericJoinPlan& plan)
    {
        for (auto& workspace : m_generic_join_workspaces)
            if (workspace.matches(plan))
                return workspace;
        return m_generic_join_workspaces.emplace_back(plan);
    }
    auto& get_database_workspace() noexcept { return m_database_workspace; }
    auto& get_values() noexcept { return m_values; }
    [[nodiscard]] const ygg::FreeRelationRepository& get_layer() const noexcept { return *m_layer; }
    /// The evaluator of a Datalog program of a constructor repository over a formalism repository in full evaluation,
    /// made by make() at first use. References stay valid while this workspace lives.
    template<std::invocable Make>
        requires std::same_as<std::invoke_result_t<Make&>, DatalogFeature>
    DatalogFeature& get_datalog_feature(const std::shared_ptr<tyr::formalism::planning::Repository>& formalism,
                                        const ConstructorRepository& constructors,
                                        ygg::uint_t program,
                                        Make&& make)
    { return find_or_make<DatalogFeature>(m_datalog_features, m_datalog_feature_positions, formalism, constructors, program, std::forward<Make>(make)); }
};

}

#endif
