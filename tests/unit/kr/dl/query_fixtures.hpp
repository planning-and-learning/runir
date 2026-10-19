#ifndef RUNIR_TESTS_KR_DL_QUERY_FIXTURES_HPP_
#define RUNIR_TESTS_KR_DL_QUERY_FIXTURES_HPP_

#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <cista/serialization.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <limits>
#include <optional>
#include <runir/kr/dl/detail/repository_instantiations_base.hpp>
#include <runir/kr/dl/detail/repository_instantiations_ext.hpp>
#include <runir/kr/dl/detail/repository_instantiations_uns.hpp>
#include <runir/kr/dl/query_data.hpp>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/detail/evaluation_helpers.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/database/semantics/operations.hpp>
#include <yggdrasil/execution/onetbb.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::tests
{
namespace
{
using Ext = kr::ExtFamilyTag;
using ObjectIndex = ygg::Index<tyr::formalism::Object>;
using QueryValues = kr::dl::QueryValues;
using ColumnIndex = ygg::Index<ygg::database::Column>;

/// The raw object indices of a relation row.
std::vector<ObjectIndex> object_values(const auto& row)
{
    auto result = std::vector<ObjectIndex> {};
    for (size_t i = 0; i < row.size(); ++i)
        result.push_back(row.template get<ObjectIndex>(i));
    return result;
}

/// The column labels of a schema.
std::vector<ColumnIndex> column_labels(const auto& columns)
{
    return ygg::database::column_labels(columns.span());
}
static_assert(!std::same_as<ColumnIndex, ygg::Index<tyr::formalism::Object>>);
static_assert(!std::is_convertible_v<ygg::Index<tyr::formalism::Object>, ColumnIndex>);

template<tyr::TaskKind Kind>
auto query_search_context()
{
    const auto directory = std::filesystem::path(__FILE__).parent_path() / "../../../fixtures/kr/dl/query";
    if constexpr (std::same_as<Kind, tyr::GroundTag>)
        return make_ground_context(directory / "domain.pddl", directory / "task.pddl");
    else
        return make_lifted_context(directory / "domain.pddl", directory / "task.pddl");
}

auto parse_query(const std::string& expression, tyr::formalism::planning::DomainView domain, kr::dl::ConstructorRepositoryFor<Ext>& repository)
{
    const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", domain, repository);
    return count.get_variant().template get<ygg::Index<kr::dl::Numerical<Ext, kr::dl::CountTag>>>().get_arg().template get<ygg::Index<kr::dl::Query<Ext>>>();
}

template<typename Data, typename Check>
void check_relocated_data(const Data& data, Check check)
{
    auto relocated = [&]
    {
        const auto original = cista::serialize(data);
        auto copy = original;
        EXPECT_NE(copy.data(), original.data());
        return copy;
    }();
    const auto* decoded = cista::deserialize<Data>(relocated);
    EXPECT_EQ(*decoded, data);
    EXPECT_EQ(decoded->index, data.index);
    check(*decoded);
}

template<typename Context>
auto persistent_context(Context& source,
                        kr::dl::semantics::DenotationCaches<Ext>& caches,
                        kr::dl::semantics::DenotationRepository& repository,
                        kr::dl::semantics::EvaluationStorage<Ext>& intermediates)
{
    return kr::dl::semantics::StateEvaluationContext<Ext,
                                                     typename Context::KindType,
                                                     std::remove_cvref_t<decltype(source.get_state())>,
                                                     std::remove_cvref_t<decltype(source.registers())>>(source.get_state(),
                                                                                                        source.get_builder(),
                                                                                                        caches,
                                                                                                        repository,
                                                                                                        intermediates,
                                                                                                        source.arguments(),
                                                                                                        source.registers());
}

}  // namespace
}  // namespace runir::tests

#endif
