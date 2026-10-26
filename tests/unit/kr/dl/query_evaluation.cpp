#include "planning_fixtures.hpp"

#include <algorithm>
#include <array>
#include <cista/serialization.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <limits>
#include <optional>
#include <runir/kr/dl/detail/repository_instantiations.hpp>
#include <runir/kr/dl/object_rows.hpp>
#include <runir/kr/dl/query_data.hpp>
#include <runir/kr/dl/query_view.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/detail/evaluation_helpers.hpp>
#include <runir/kr/dl/semantics/detail/repository_instantiations.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/state_evaluation.hpp>
#include <runir/kr/dl/semantics/state_evaluation_context.hpp>
#include <runir/kr/ps/ext/dl/parser.hpp>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/planning/interning.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/database/operators/relational/join.hpp>
#include <yggdrasil/database/operators/relational/project.hpp>
#include <yggdrasil/execution/onetbb.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::tests
{
namespace
{
using ObjectIndex = ygg::Index<tyr::formalism::Object>;
using ColumnIndex = ygg::Index<ygg::database::Column>;

/// The raw object indices of a relation row.
std::vector<ObjectIndex> object_values(const auto& row)
{
    auto result = std::vector<ObjectIndex> {};
    for (size_t i = 0; i < row.size(); ++i)
        result.push_back(ObjectIndex(row.template get<ygg::uint_t>(i)));
    return result;
}

/// The column labels of a schema.
std::vector<ColumnIndex> column_labels(const auto& columns) { return ygg::database::column_labels(columns.span()); }
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

auto parse_query(const std::string& expression, tyr::formalism::planning::DomainView domain, kr::dl::ConstructorRepository& repository)
{
    const auto count = kr::ps::ext::dl::parse_numerical("(n_count " + expression + ")", domain, repository);
    return count.get_variant().template get<ygg::Index<kr::dl::Numerical<kr::dl::CountTag>>>().get_arg().template get<ygg::Index<kr::dl::Query<>>>();
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
                        kr::dl::semantics::DenotationCaches& caches,
                        kr::dl::semantics::DenotationRepository& repository,
                        kr::dl::semantics::EvaluationStorage& intermediates)
{
    return kr::dl::semantics::StateEvaluationContext<typename Context::KindType, std::remove_cvref_t<decltype(source.get_state())>>(source.get_state(),
                                                                                                                                    source.get_builder(),
                                                                                                                                    caches,
                                                                                                                                    repository,
                                                                                                                                    intermediates,
                                                                                                                                    source.arguments(),
                                                                                                                                    source.registers());
}

template<tyr::TaskKind Kind>
void check_queries()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(),
                                                                   builder,
                                                                   storage,
                                                                   kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                   kr::dl::semantics::insert(denotations, registers).first);

    const auto triple = std::string(R"((q_atomic_state "triple" (x y z)))");
    const auto fixed = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto ready = std::string(R"((q_atomic_state "ready" ()))");
    const auto missing = std::string(R"((q_atomic_state "missing" ()))");
    const auto count = [&](const std::string& query)
    {
        const auto expression = kr::ps::ext::dl::parse_numerical("(n_count " + query + ")", domain, *repository);
        return kr::dl::semantics::evaluate<Kind>(expression, context).get();
    };
    const std::vector<std::pair<std::string, ygg::uint_t>> cases {
        { triple, 4 },
        { fixed, 2 },
        { R"((q_atomic_state "copied" (x y z)))", 4 },
        { R"((q_atomic_state "assignment" (x y z w)))", 4 },
        { R"((q_project (x) (q_join (q_atomic_state "assignment" (x y z w)) (q_atomic_state "required" (x y z)))))", 1 },
        { ready, 1 },
        { missing, 0 },
        { R"((q_atomic_goal "triple" true (x y z)))", 1 },
        { R"((q_atomic_goal "triple" false (x y z)))", 1 },
        { "(q_project (x) " + triple + ")", 2 },
        { "(q_project (y x) " + triple + ")", 3 },
        { "(q_project () " + triple + ")", 1 },
        { "(q_project () " + missing + ")", 0 },
        { "(q_select_equal y z " + triple + ")", 2 },
        { "(q_select_equal x y " + triple + ")", 1 },
        { "(q_select_value x \"a\" " + triple + ")", 2 },
        { "(q_join " + triple + " " + fixed + ")", 1 },
        { "(q_generic_join (x y z) (x y z) " + triple + " " + fixed + ")", 1 },
        { "(q_generic_join (y x z) (z x y) " + triple + R"( (q_role (x y) (r_atomic_state "edge"))))", 3 },
        { R"((q_generic_join (x y z) (x y z) (q_role (x y) (r_atomic_state "edge")) (q_role (y z) (r_atomic_state "edge")) (q_role (x z) (r_atomic_state "edge"))))",
          0 },
        { "(q_empty (x y))", 0 },
        { "(q_join " + triple + " (q_empty (x)))", 0 },
        { "(q_union " + triple + " " + fixed + ")", 5 },
        { "(q_difference " + triple + " " + fixed + ")", 3 },
        { "(q_join " + triple + " " + ready + ")", 4 },
        { "(q_join " + triple + " " + missing + ")", 0 },
        { "(q_join (q_rename (u v w) " + triple + R"() (q_atomic_state "marker" (u))))", 2 },
        { "(q_join " + triple + R"( (q_concept x (c_nominal "a"))))", 2 },
        { "(q_join " + triple + R"( (q_role (x y) (r_atomic_state "edge"))))", 3 },
        { R"((q_role (x y) (r_and (r_atomic_state "edge") (r_inverse (r_atomic_state "edge")))))", 0 },
        { R"((q_role (x y) (r_or (r_atomic_state "edge") (r_inverse (r_atomic_state "edge")))))", 4 },
        { R"((q_role (x y) (r_complement (r_atomic_state "edge"))))", 7 },
        { R"((q_role (x y) (r_or (r_atomic_state "edge") (r_complement (r_atomic_state "edge")))))", 9 },
        { R"((q_join (q_join (q_atomic_state "marker" (a)) (q_atomic_state "marker" (b)))
                        (q_join (q_atomic_state "marker" (c)) (q_atomic_state "marker" (d)))))",
          16 },
        { "(q_concept x (c_project z " + triple + "))", 3 },
        // Every path a -> b -> c passes all three objects; keyed by each marker (a, c), the same graph twice.
        { R"((q_must_pass () (q_concept x (c_nominal "a")) (q_role (x y) (r_atomic_state "edge")) (q_concept x (c_nominal "c")) (q_concept p (c_top))))", 3 },
        // Sources, targets and probes may use closures (only the edges are search-free): the same three objects.
        { R"((q_must_pass () (q_concept x (c_and (c_nominal "a") (c_some (r_transitive_closure (r_atomic_state "edge")) (c_top))))
                             (q_role (x y) (r_atomic_state "edge"))
                             (q_concept x (c_and (c_nominal "c") (c_some (r_inverse (r_transitive_closure (r_atomic_state "edge"))) (c_top))))
                             (q_concept p (c_some (r_reflexive_transitive_closure (r_atomic_state "edge")) (c_nominal "c")))))",
          3 },
        { R"((q_must_pass (k) (q_join (q_atomic_state "marker" (k)) (q_concept v (c_nominal "a")))
                              (q_join (q_atomic_state "marker" (k)) (q_role (u v) (r_atomic_state "edge")))
                              (q_join (q_atomic_state "marker" (k)) (q_concept v (c_nominal "c")))
                              (q_concept p (c_top))))",
          6 },
        { R"((q_must_pass () (q_concept x (c_nominal "c")) (q_role (x y) (r_atomic_state "edge")) (q_concept x (c_nominal "a")) (q_concept p (c_top))))", 0 },
        // Per key x of triple(x, y, z) as edges y -> z: key a has b -> b, b -> c, from {b} to {b, c}: only b; key b has
        // c -> a, b -> b, from {c, b} to {a, b}: no object is on every path.
        { "(q_must_pass (x) (q_project (x y) " + triple + ") " + triple + " (q_project (x z) " + triple + R"() (q_concept p (c_top))))", 1 },
    };
    // Reevaluate different schemas/arity and empty outputs using the retained workspace.
    for (int repeat = 0; repeat < 3; ++repeat)
    {
        storage.reset_all();
        for (const auto& [query, expected] : cases)
        {
            SCOPED_TRACE(query);
            EXPECT_EQ(count(query), expected);
            EXPECT_EQ(count(query), expected);
        }
    }

    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_boolean("(b_nonempty " + ready + ")", domain, *repository), context).get());
    EXPECT_FALSE(kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_boolean("(b_nonempty " + missing + ")", domain, *repository), context).get());
    const auto concept_ = kr::ps::ext::dl::parse_concept("(c_project z " + triple + ")", domain, *repository);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(concept_, context).get().count(), 3);
    const auto role = kr::ps::ext::dl::parse_role("(r_project y x " + triple + ")", domain, *repository);
    const auto reversed = kr::dl::semantics::evaluate<Kind>(role, context);
    EXPECT_EQ(reversed.count(), 3);
    const auto a = domain.get_constants()[0];
    const auto b = domain.get_constants()[1];
    const auto c = domain.get_constants()[2];
    ASSERT_EQ(a.get_name(), "a");
    ASSERT_EQ(b.get_name(), "b");
    ASSERT_EQ(c.get_name(), "c");
    EXPECT_TRUE(reversed.get(b.get_index())[ygg::uint_t(a.get_index())]);
    EXPECT_TRUE(reversed.get(c.get_index())[ygg::uint_t(b.get_index())]);
    EXPECT_FALSE(reversed.get(a.get_index())[ygg::uint_t(b.get_index())]);

    // a has a matching y and a matching z in different rows; only b has one complete witness.
    const auto correlated =
        kr::ps::ext::dl::parse_concept(R"((c_project x (q_join (q_atomic_state "assignment" (x y z w)) (q_atomic_state "required" (x y z)))))",
                                       domain,
                                       *repository);
    const auto witnesses = kr::dl::semantics::evaluate<Kind>(correlated, context).get();
    EXPECT_EQ(witnesses.count(), 1);
    EXPECT_TRUE(witnesses[ygg::uint_t(b.get_index())]);
    EXPECT_FALSE(witnesses[ygg::uint_t(a.get_index())]);

    const auto evaluate_register = [&](auto expression)
    {
        auto updated = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(),
                                                                       builder,
                                                                       storage,
                                                                       kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                       kr::dl::semantics::insert(denotations, registers).first);
        return kr::dl::semantics::evaluate<Kind>(expression, updated);
    };
    const auto register_count = kr::ps::ext::dl::parse_numerical("(n_count (q_join " + triple + " (q_concept x (c_register 0))))", domain, *repository);
    EXPECT_EQ(evaluate_register(register_count).get(), 0);
    registers.concept_values[0] = a.get_index();
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(register_count).get(), 2);
    registers.concept_values[0] = c.get_index();
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(register_count).get(), 0);
    registers.concept_values[0].reset();
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(register_count).get(), 0);

    const auto role_register_count = kr::ps::ext::dl::parse_numerical("(n_count (q_join " + triple + " (q_role (x y) (r_register 0))))", domain, *repository);
    registers.role_values[0] = ::cista::pair(a.get_index(), b.get_index());
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(role_register_count).get(), 2);
    registers.role_values[0] = ::cista::pair(b.get_index(), c.get_index());
    storage.reset_dynamic();
    EXPECT_EQ(evaluate_register(role_register_count).get(), 1);

    EXPECT_FALSE(context.registers().template get<kr::dl::ConceptTag>()[0]);
    EXPECT_FALSE(context.registers().template get<kr::dl::RoleTag>()[0]);

    auto persistent_memo = kr::dl::semantics::DenotationCaches {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto a_set = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", domain, *repository), persistent);
    const auto c_set = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(R"((c_nominal "c"))", domain, *repository), persistent);
    auto argument_values = ygg::Data<kr::dl::semantics::CallArguments>();
    argument_values.concept_arguments.push_back(a_set.get_index());
    auto argument_context = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(),
                                                                            builder,
                                                                            storage,
                                                                            kr::dl::semantics::insert(denotations, argument_values).first,
                                                                            kr::dl::semantics::insert(denotations, registers).first);
    const auto argument_count = kr::ps::ext::dl::parse_numerical("(n_count (q_join " + triple + " (q_concept x (c_argument 0))))", domain, *repository);
    // An invalid Ext argument must propagate through DL/query variant dispatch, not terminate.
    EXPECT_THROW(kr::dl::semantics::evaluate<Kind>(argument_count, context), std::invalid_argument);
    // A context that binds no arguments or registers rejects both.
    auto unbound_context = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(), builder, storage);
    EXPECT_FALSE(unbound_context.has_arguments());
    EXPECT_THROW(kr::dl::semantics::evaluate<Kind>(argument_count, unbound_context), std::invalid_argument);
    EXPECT_THROW(kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept("(c_register 0)", domain, *repository), unbound_context),
                 std::invalid_argument);
    // Ext programs also use the comparisons, constants and connectives.
    const auto parse_boolean = [&](const std::string& description) { return kr::ps::ext::dl::parse_boolean(description, domain, *repository); };
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(parse_boolean("(b_and (b_const true) (b_not (n_lt (n_const 2) (n_const 1))))"), unbound_context).get());
    EXPECT_FALSE(
        kr::dl::semantics::evaluate<Kind>(parse_boolean("(b_or (b_const false) (n_eq (n_add (n_const 1) (n_const 1)) (n_const 3)))"), unbound_context).get());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(parse_boolean("(b_neq (b_nonempty (c_top)) (b_const false))"), unbound_context).get());
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument_count, argument_context).get(), 2);
    const auto argument = kr::ps::ext::dl::parse_concept("(c_argument 0)", domain, *repository);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument, argument_context), a_set);
    argument_values.concept_arguments[0] = c_set.get_index();
    auto other_argument_context = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(),
                                                                                  builder,
                                                                                  storage,
                                                                                  kr::dl::semantics::insert(denotations, argument_values).first,
                                                                                  kr::dl::semantics::insert(denotations, registers).first);
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument_count, other_argument_context).get(), 0);
    EXPECT_EQ(argument_context.arguments().template get<kr::dl::ConceptTag>()[0].get_index(), a_set.get_index());

    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    auto next = kr::dl::semantics::StateEvaluationContext<Kind>(successors.front().get_state(),
                                                                builder,
                                                                storage,
                                                                kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                kr::dl::semantics::insert(denotations, registers).first);
    const auto triple_count = kr::ps::ext::dl::parse_numerical("(n_count " + triple + ")", domain, *repository);
    const auto derived_count = kr::ps::ext::dl::parse_numerical(R"((n_count (q_atomic_state "copied" (x y z))))", domain, *repository);
    const auto ready_test = kr::ps::ext::dl::parse_boolean("(b_nonempty " + ready + ")", domain, *repository);
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(triple_count, next).get(), 3);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(derived_count, next).get(), 3);
    EXPECT_FALSE(kr::dl::semantics::evaluate<Kind>(ready_test, next).get());
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(triple_count, context).get(), 4);
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(ready_test, context).get());

    // Borrowed state/register views use interned arguments, which survive feature-cache invalidation.
    {
        auto argument_data = ygg::Data<kr::dl::semantics::CallArguments> {};
        argument_data.concept_arguments.push_back(a_set.get_index());
        const auto arguments = kr::dl::semantics::insert(denotations, argument_data).first;
        const auto& planning_state = initial.get_state();
        auto borrowed_context = kr::dl::semantics::StateEvaluationContext<Kind, tyr::planning::BuilderStateView<Kind>>(
            tyr::planning::BuilderStateView<Kind>(planning_state.get_state_builder(), *search->task),
            builder,
            storage,
            arguments,
            ygg::make_view(registers, *search->task->get_repository()));
        static_assert(std::is_same_v<decltype(borrowed_context.arguments()), kr::dl::semantics::CallArgumentsView>);
        EXPECT_EQ(&borrowed_context.arguments().get_data(), &arguments.get_data());
        auto copied_context = borrowed_context;
        EXPECT_EQ(&copied_context.arguments().get_data(), &arguments.get_data());
        storage.reset_dynamic();
        EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument, borrowed_context), a_set);
        EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(), 0);
        storage.reset_dynamic();
        EXPECT_TRUE(a_set.get()[ygg::uint_t(a.get_index())]);
        EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument, borrowed_context), a_set);
        storage.reset_dynamic();
    }

    // Argument nodes forward their durable identity, while computed roots use the selected output binding.
    auto destination = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto destination_memo = kr::dl::semantics::DenotationCaches {};
    auto argument_output = persistent_context(argument_context, destination_memo, destination, storage);
    const auto retained_argument = kr::dl::semantics::evaluate<Kind>(argument, argument_output);
    EXPECT_EQ(retained_argument, a_set);
    EXPECT_EQ(destination.template size<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(), 0);
    const auto distance = kr::ps::ext::dl::parse_numerical(R"((n_distance (c_nominal "a") (r_atomic_state "edge") (c_nominal "c")))", domain, *repository);
    auto computed_output = persistent_context(context, destination_memo, destination, storage);
    const auto retained_distance = kr::dl::semantics::evaluate<Kind>(distance, computed_output);
    EXPECT_EQ(&retained_distance.get_context(), &destination);
    EXPECT_EQ(destination.template size<kr::dl::semantics::Denotation<kr::dl::ConceptTag>>(), 0);
    EXPECT_EQ(destination.template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    storage.reset_all();
    EXPECT_EQ(retained_argument.get().count(), 1);
    EXPECT_TRUE(retained_argument.get()[ygg::uint_t(a.get_index())]);
    denotations.reset();
    EXPECT_EQ(retained_distance.get(), 2);
}

/// Distance arguments may be concepts/roles or queries; every form shares one evaluation.
template<tyr::TaskKind Kind>
void check_distance_queries()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    auto context = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(),
                                                                   builder,
                                                                   storage,
                                                                   kr::dl::semantics::insert(denotations, arguments).first,
                                                                   kr::dl::semantics::insert(denotations, registers).first);
    // The previous evaluation: min_distance over fully evaluated sources, edges, and targets.
    const auto reference = [&](const std::string& expression)
    {
        const auto numerical = kr::ps::ext::dl::parse_numerical(expression, domain, *repository);
        return ygg::visit(
            [&]<typename Concrete>(Concrete concrete) -> ygg::uint_t
            {
                if constexpr (requires { concrete.get_mid(); })
                {
                    const auto& plan = concrete.get_data().plan;
                    auto children = context.child_context();
                    const auto relation = [&](auto argument, auto columns)
                    {
                        return ygg::visit([&](auto child) { return kr::dl::semantics::detail::evaluate_relation<Kind>(child, columns.span(), children); },
                                          argument);
                    };
                    const auto sources = relation(concrete.get_lhs(), plan.source_columns());
                    const auto edges = relation(concrete.get_mid(), plan.edge_columns());
                    const auto targets = relation(concrete.get_rhs(), plan.target_columns());
                    auto distances = ygg::Builder<ygg::database::Relation>(plan.output_columns().span());
                    auto workspace = ygg::database::DistanceWorkspace(plan);
                    return ygg::database::min_distance(sources, edges, targets, plan, distances, workspace);
                }
                else
                    throw std::logic_error("reference expects n_distance");
            },
            numerical.get_variant());
    };
    const auto number = [&](const std::string& expression)
    {
        const auto value = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_numerical(expression, domain, *repository), context).get();
        EXPECT_EQ(value, reference(expression)) << expression;
        return value;
    };
    constexpr auto infinity = std::numeric_limits<ygg::uint_t>::max();

    // edge: a -> b -> c.
    const auto edge = std::string(R"((r_atomic_state "edge"))");
    const auto edges = "(q_role (x y) " + edge + ")";
    const auto vertex = [](const char* name) { return "(q_concept x (c_nominal \"" + std::string(name) + "\"))"; };
    EXPECT_EQ(number("(n_distance " + vertex("a") + " " + edges + " " + vertex("c") + ")"), 2);
    EXPECT_EQ(number("(n_distance (c_nominal \"a\") " + edges + " (c_nominal \"c\"))"), 2);
    EXPECT_EQ(number("(n_distance " + vertex("a") + " " + edge + " (c_nominal \"b\"))"), 1);
    EXPECT_EQ(number("(n_distance " + vertex("c") + " " + edges + " " + vertex("a") + ")"), infinity);
    EXPECT_EQ(number("(n_distance " + vertex("b") + " " + edges + " " + vertex("b") + ")"), 0);
    EXPECT_EQ(number("(n_distance (q_concept x (c_bot)) " + edges + " " + vertex("b") + ")"), infinity);
    // Sources and targets may use closures (only the edges are search-free): the objects with a successor are {a, b},
    // those reaching c are {a, b, c}.
    EXPECT_EQ(number("(n_distance (c_some (r_transitive_closure " + edge + ") (c_top)) " + edges + " (c_nominal \"c\"))"), 1);
    EXPECT_EQ(number("(n_distance (c_nominal \"a\") " + edge + " (c_some (r_reflexive_transitive_closure " + edge + ") (c_nominal \"c\")))"), 0);

    // k = 2: both tuple fields step along edges simultaneously, (x u) -> (y v).
    const auto pair_edges = "(q_project (x u y v) (q_join " + edges + " (q_role (u v) " + edge + ")))";
    const auto pair = [](const char* name) { return "(q_role (x u) (r_identity (c_nominal \"" + std::string(name) + "\")))"; };
    EXPECT_EQ(number("(n_distance " + pair("a") + " " + pair_edges + " " + pair("c") + ")"), 2);
    EXPECT_EQ(number("(n_distance " + pair("c") + " " + pair_edges + " " + pair("a") + ")"), infinity);
    EXPECT_EQ(number("(n_distance " + pair("c") + " " + pair_edges + " " + pair("c") + ")"), 0);

    // Bound evaluation through every query form: renames, selections, unions, differences, role leaves with one
    // or both columns bound (including identities and complements), and joins without shared columns.
    const auto all = std::string("(q_concept x (c_top))");
    const auto renamed = "(q_rename (x y) " + edges + ")";
    EXPECT_EQ(number("(n_distance " + vertex("a") + " " + renamed + " " + vertex("c") + ")"), 2);
    const auto reversed = "(q_project (x y) (q_rename (y x) " + edges + "))";
    EXPECT_EQ(number("(n_distance " + vertex("c") + " " + reversed + " " + vertex("a") + ")"), 2);
    const auto both_ways = "(q_union " + edges + " " + reversed + ")";
    EXPECT_EQ(number("(n_distance " + vertex("c") + " " + both_ways + " " + vertex("a") + ")"), 2);
    const auto without_ab = "(q_difference " + edges + " (q_role (x y) (r_restriction (r_identity (c_nominal \"a\")) (c_top))))";
    EXPECT_EQ(number("(n_distance " + vertex("a") + " (q_difference " + edges + " (q_join (q_concept x (c_nominal \"a\")) (q_concept y (c_nominal \"b\")))) "
                     + vertex("c") + ")"),
              infinity);
    EXPECT_EQ(number("(n_distance " + vertex("b") + " " + without_ab + " " + vertex("c") + ")"), 1);
    const auto any_other = "(q_role (x y) (r_complement (r_identity (c_top))))";
    EXPECT_EQ(number("(n_distance " + vertex("c") + " " + any_other + " " + vertex("a") + ")"), 1);
    const auto inverse_target_bound = "(q_project (x y) (q_join (q_role (y x) " + edge + ") (q_concept x (c_top))))";
    EXPECT_EQ(number("(n_distance " + vertex("c") + " " + inverse_target_bound + " " + vertex("a") + ")"), 2);
    const auto loops = "(q_select_equal x y (q_union " + edges + " (q_role (x y) (r_identity (c_top)))))";
    EXPECT_EQ(number("(n_distance " + vertex("a") + " " + loops + " " + vertex("c") + ")"), infinity);
    const auto product = "(q_join (q_concept x (c_nominal \"a\")) (q_concept y (c_top)))";
    EXPECT_EQ(number("(n_distance " + vertex("a") + " " + product + " " + vertex("c") + ")"), 1);
    EXPECT_EQ(number("(n_distance " + all + " " + edges + " " + vertex("c") + ")"), 0);

    // k = 2 Sokoban-style walk/push edges: one coordinate moves, the other stays put through an identity, and
    // the mover may not enter the other's cell.
    const auto walk =
        "(q_project (x u y v) (q_join (q_join " + edges + " (q_role (u v) (r_identity (c_top)))) (q_role (y u) (r_complement (r_identity (c_top))))))";
    const auto push = "(q_project (x u y v) (q_join (q_join (q_role (x u) " + edge + ") (q_role (u v) " + edge + ")) (q_role (u y) (r_identity (c_top)))))";
    const auto moves = "(q_union " + walk + " " + push + ")";
    const auto at = [](const char* x, const char* u)
    { return "(q_join (q_concept x (c_nominal \"" + std::string(x) + "\")) (q_concept u (c_nominal \"" + std::string(u) + "\")))"; };
    const auto box_at_c = "(q_role (x u) (r_restriction (r_universal) (c_nominal \"c\")))";
    EXPECT_EQ(number("(n_distance " + at("a", "b") + " " + moves + " " + box_at_c + ")"), 1);
    EXPECT_EQ(number("(n_distance " + at("b", "a") + " " + moves + " " + box_at_c + ")"), infinity);
    EXPECT_EQ(number("(n_distance " + at("a", "c") + " " + moves + " " + box_at_c + ")"), 0);
    // The complement nested in a join of its own: the translation flattens the conjuncts to make it an antijoin.
    const auto nested_walk =
        "(q_project (x u y v) (q_join " + edges + " (q_join (q_role (u v) (r_identity (c_top))) (q_role (y u) (r_complement (r_identity (c_top)))))))";
    const auto nested_moves = "(q_union " + nested_walk + " " + push + ")";
    EXPECT_EQ(number("(n_distance " + at("a", "b") + " " + nested_moves + " " + box_at_c + ")"), 1);
    EXPECT_EQ(number("(n_distance " + at("b", "a") + " " + nested_moves + " " + box_at_c + ")"), infinity);

    // A* relaxes dynamic leaves away: `triple` is dynamic, so the heuristic ignores which x may move,
    // while the search still respects it. x = a has triples (a b b) and (a b c); only x = b has (b c a).
    const auto with_triple = [&](const char* middle)
    {
        return "(q_project (x u y v) (q_join " + pair_edges + " (q_project (x) (q_select_value m \"" + std::string(middle)
               + "\" (q_atomic_state \"triple\" (x m n))))))";
    };
    EXPECT_EQ(number("(n_distance " + pair("a") + " " + with_triple("b") + " " + pair("c") + ")"), 2);
    EXPECT_EQ(number("(n_distance " + pair("a") + " " + with_triple("c") + " " + pair("c") + ")"), infinity);
    EXPECT_EQ(number("(n_distance " + pair("b") + " " + with_triple("c") + " " + pair("c") + ")"), 1);

    // Insertion requires arities k, 2k, k.
    using Distance = ygg::Data<kr::dl::Numerical<kr::dl::DistanceTag>>;
    const auto source = kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", domain, *repository);
    const auto role = kr::ps::ext::dl::parse_role(edge, domain, *repository);
    auto mismatched = Distance(source.get_index(), role.get_index(), parse_query(pair("c"), domain, *repository).get_index());
    EXPECT_THROW(static_cast<void>(kr::dl::insert(*repository, mismatched)), std::invalid_argument);
    mismatched = Distance(source.get_index(), parse_query(pair_edges, domain, *repository).get_index(), source.get_index());
    EXPECT_THROW(static_cast<void>(kr::dl::insert(*repository, mismatched)), std::invalid_argument);
}

void check_cached_queries()
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    auto factory = kr::dl::ConstructorRepositoryFactory();
    auto repository = factory.create(search->task->get_repository());
    auto other_repository = factory.create(search->task->get_repository());
    auto& repo = *repository;
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto& caches = storage.get_caches();
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(state, builder, storage);

    auto column_data = ygg::Data<kr::dl::QueryColumn>();
    column_data.name = "x";
    const auto x = kr::dl::insert(repo, column_data).first.get_index();
    column_data.name = "renamed";
    const auto renamed_column = kr::dl::insert(repo, column_data).first.get_index();
    auto top_data = ygg::Data<kr::dl::Concept<kr::dl::TopTag>>();
    auto top_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::ConceptTag>>(kr::dl::insert(repo, top_data).first.get_index());
    const auto top = kr::dl::insert(repo, top_wrapper).first;
    using Lift = kr::dl::Query<kr::dl::QueryConceptTag>;
    auto lift_data = ygg::Data<Lift>();
    lift_data.arg = top.get_index();
    EXPECT_THROW(static_cast<void>(kr::dl::insert(repo, lift_data)), std::invalid_argument);
    EXPECT_EQ(repo.template size<Lift>(), 0);
    lift_data.columns.push_back(x);
    const auto [lift, created] = kr::dl::insert(repo, lift_data);
    EXPECT_TRUE(created);
    const auto [duplicate_lift, duplicate_created] = kr::dl::insert(repo, lift_data);
    EXPECT_FALSE(duplicate_created);
    EXPECT_EQ(duplicate_lift.get_index(), lift.get_index());
    EXPECT_EQ(repo.template size<Lift>(), 1);
    auto query_data = ygg::Data<kr::dl::Query<>>();
    query_data.variant = lift.get_index();
    const auto query = kr::dl::insert(repo, query_data).first;
    auto rename_data = ygg::Data<kr::dl::Query<kr::dl::QueryRenameTag>>();
    rename_data.arg = query.get_index();
    rename_data.columns.push_back(renamed_column);
    query_data.variant = kr::dl::insert(repo, rename_data).first.get_index();
    const auto renamed = kr::dl::insert(repo, query_data).first;

    auto expected_columns = ygg::IndexList<kr::dl::QueryColumn>();
    expected_columns.push_back(renamed_column);
    expected_columns.push_back(x);
    const auto check_inferred_schema = [&](auto& data)
    {
        const auto concrete = kr::dl::insert(repo, data).first;
        EXPECT_EQ(concrete.get_data().columns, expected_columns);

        check_relocated_data(concrete.get_data(),
                             [&](const auto& decoded)
                             {
                                 EXPECT_EQ(decoded.columns, expected_columns);
                                 if constexpr (requires { decoded.schema; })
                                 {
                                     EXPECT_TRUE(std::ranges::equal(ygg::make_view(decoded.schema, repo), concrete.get_schema()));
                                 }
                                 if constexpr (requires { data.plan; })
                                 {
                                     EXPECT_TRUE(std::ranges::equal(decoded.plan.output_columns(), concrete.get_schema()));
                                     EXPECT_TRUE(std::ranges::equal(decoded.plan.lhs_keys(), data.plan.lhs_keys()));
                                     EXPECT_TRUE(std::ranges::equal(decoded.plan.rhs_keys(), data.plan.rhs_keys()));
                                     EXPECT_TRUE(std::ranges::equal(decoded.plan.rhs_payload(), data.plan.rhs_payload()));
                                 }
                                 if constexpr (requires { decoded.lhs_position; })
                                 {
                                     EXPECT_EQ(decoded.lhs_position, 1);
                                     EXPECT_EQ(decoded.rhs_position, 0);
                                 }
                                 if constexpr (requires { decoded.position; })
                                 {
                                     EXPECT_EQ(decoded.position, 1);
                                 }
                             });

        // Derived columns, plans and positions neither change identity nor override child schemas.
        auto stale = data;
        stale.columns.clear();
        stale.columns.push_back(x);
        stale.columns.push_back(x);
        if constexpr (requires { stale.schema; })
            stale.schema.clear();
        if constexpr (requires { stale.plan; })
            stale.plan = {};
        if constexpr (requires { stale.lhs_position; })
            stale.lhs_position = stale.rhs_position = 99;
        if constexpr (requires { stale.position; })
            stale.position = 99;
        EXPECT_EQ(stale, concrete.get_data());
        const auto [same, stale_created] = ygg::formalism::insert(repo, stale);
        EXPECT_FALSE(stale_created);
        EXPECT_EQ(same.get_index(), concrete.get_index());
        EXPECT_EQ(stale.columns, expected_columns);
        if constexpr (requires { stale.schema; })
        {
            EXPECT_TRUE(std::ranges::equal(ygg::make_view(stale.schema, repo), concrete.get_schema()));
        }
        if constexpr (requires { stale.plan; })
        {
            EXPECT_TRUE(std::ranges::equal(stale.plan.output_columns(), concrete.get_schema()));
        }
        if constexpr (requires { stale.lhs_position; })
        {
            EXPECT_EQ(stale.lhs_position, 1);
            EXPECT_EQ(stale.rhs_position, 0);
        }
        if constexpr (requires { stale.position; })
        {
            EXPECT_EQ(stale.position, 1);
        }
        stale.clear();
        if constexpr (requires { stale.schema; })
        {
            EXPECT_TRUE(stale.schema.empty());
        }
        if constexpr (requires { stale.plan; })
        {
            EXPECT_TRUE(stale.plan.output_columns().empty());
        }
        if constexpr (requires { stale.lhs_position; })
        {
            EXPECT_EQ(stale.lhs_position, 0);
            EXPECT_EQ(stale.rhs_position, 0);
        }
        if constexpr (requires { stale.position; })
        {
            EXPECT_EQ(stale.position, 0);
        }

        query_data.variant = concrete.get_index();
        const auto wrapper = kr::dl::insert(repo, query_data).first;
        EXPECT_TRUE(std::ranges::equal(wrapper.get_columns(), expected_columns, {}, [](auto column) { return column.get_index(); }));
        return wrapper;
    };
    auto join_data = ygg::Data<kr::dl::Query<kr::dl::QueryJoinTag>>();
    join_data.lhs = renamed.get_index();
    join_data.rhs = query.get_index();
    const auto joined = check_inferred_schema(join_data);
    join_data.lhs = joined.get_index();
    check_inferred_schema(join_data);  // The shared x column is not appended twice.
    check_relocated_data(join_data,
                         [&](const auto& decoded)
                         {
                             ygg::Builder<ygg::database::Relation> lhs(decoded.plan.lhs_columns().span()), rhs(decoded.plan.rhs_columns().span()),
                                 result(decoded.plan.output_columns().span());
                             lhs.insert(std::tuple { static_cast<ygg::uint_t>(10), static_cast<ygg::uint_t>(20) });
                             lhs.insert(std::tuple { static_cast<ygg::uint_t>(30), static_cast<ygg::uint_t>(40) });
                             rhs.insert(std::tuple { static_cast<ygg::uint_t>(20) });
                             ygg::database::Workspace scratch;
                             ygg::database::join(lhs, rhs, decoded.plan, result, scratch);
                             EXPECT_EQ(result.size(), 1);
                             EXPECT_TRUE(result.contains(std::tuple { ygg::uint_t(10), ygg::uint_t(20) }));
                         });
    const auto slice_position = [](const ygg::database::ColumnSlice& slice) { return slice.offset / sizeof(ygg::uint_t); };
    const auto check_projection_plan = [&](auto& data, auto positions)
    {
        const auto concrete = kr::dl::insert(repo, data).first;
        check_relocated_data(concrete.get_data(),
                             [&](const auto& decoded)
                             {
                                 EXPECT_TRUE(std::ranges::equal(decoded.plan.positions(), positions, {}, slice_position));
                                 ygg::Builder<ygg::database::Relation> input(decoded.plan.input_columns().span()), result(decoded.plan.output_columns().span());
                                 std::vector<ObjectIndex> row;
                                 row.reserve(input.arity());
                                 for (size_t i = 0; i < input.arity(); ++i)
                                     row.emplace_back(10 + i);
                                 std::vector<std::byte> bytes;
                                 input.insert(kr::dl::encode_object_row(row, bytes));
                                 ygg::database::Workspace scratch;
                                 ygg::database::project(input, decoded.plan, result, scratch);
                                 ASSERT_EQ(result.size(), 1);
                                 for (size_t i = 0; i < positions.size(); ++i)
                                     EXPECT_EQ(ObjectIndex(result[0].template get<ygg::uint_t>(i)), ObjectIndex(10 + positions[i]));
                             });
        auto stale = data;
        stale.plan = {};
        EXPECT_EQ(stale, concrete.get_data());
        const auto [same, stale_created] = kr::dl::insert(repo, stale);
        EXPECT_FALSE(stale_created);
        EXPECT_EQ(same.get_index(), concrete.get_index());
        EXPECT_TRUE(std::ranges::equal(stale.plan.positions(), positions, {}, slice_position));
        stale.clear();
        EXPECT_TRUE(stale.plan.input_columns().empty());
        EXPECT_TRUE(stale.plan.output_columns().empty());
        EXPECT_TRUE(stale.plan.positions().empty());
    };
    auto project_data = ygg::Data<kr::dl::Query<kr::dl::QueryProjectTag>>();
    project_data.arg = joined.get_index();
    project_data.columns.push_back(x);
    project_data.columns.push_back(renamed_column);
    check_projection_plan(project_data, std::array<size_t, 2> { 1, 0 });
    auto role_projection_data = ygg::Data<kr::dl::QueryProjection<kr::dl::RoleTag>>();
    role_projection_data.arg = project_data.arg;
    role_projection_data.columns = project_data.columns;
    check_projection_plan(role_projection_data, std::array<size_t, 2> { 1, 0 });

    auto union_data = ygg::Data<kr::dl::Query<kr::dl::QueryUnionTag>>();
    union_data.lhs = joined.get_index();
    union_data.rhs = joined.get_index();
    check_inferred_schema(union_data);
    auto difference_data = ygg::Data<kr::dl::Query<kr::dl::QueryDifferenceTag>>();
    difference_data.lhs = joined.get_index();
    difference_data.rhs = joined.get_index();
    check_inferred_schema(difference_data);
    auto equal_data = ygg::Data<kr::dl::Query<kr::dl::QuerySelectEqualTag>>();
    equal_data.arg = joined.get_index();
    equal_data.lhs_column = x;
    equal_data.rhs_column = renamed_column;
    check_inferred_schema(equal_data);
    auto value_data = ygg::Data<kr::dl::Query<kr::dl::QuerySelectValueTag>>();
    value_data.arg = joined.get_index();
    value_data.column = x;
    value_data.object = search->task->get_domain().get_domain().get_constants()[0].get_index();
    check_inferred_schema(value_data);

    auto count_data = ygg::Data<kr::dl::Numerical<kr::dl::CountTag>>(renamed.get_index());
    auto count_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::NumericalTag>>(kr::dl::insert(repo, count_data).first.get_index());
    const auto count = kr::dl::insert(repo, count_wrapper).first;
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(count, context).get(), 3);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(count, context).get(), 3);
    EXPECT_TRUE(caches.template get<kr::dl::ConceptTag>(true).contains(top));

    auto projection_data = ygg::Data<kr::dl::QueryProjection<kr::dl::ConceptTag>>();
    projection_data.arg = renamed.get_index();
    projection_data.columns.push_back(renamed_column);
    check_projection_plan(projection_data, std::array<size_t, 1> { 0 });
    auto projection_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::ConceptTag>>(kr::dl::insert(repo, projection_data).first.get_index());
    const auto projection = kr::dl::insert(repo, projection_wrapper).first;
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(projection, context).get().count(), 3);

    const auto held = kr::dl::semantics::evaluate<tyr::GroundTag>(renamed, context);
    const auto* held_storage = held.get_storage_address();
    const auto* held_columns = held.columns().data();
    EXPECT_EQ(held.columns()[0].label, ColumnIndex(ygg::uint_t(renamed_column)));
    EXPECT_NE(held_columns, renamed.get_schema().data());
    EXPECT_TRUE(std::ranges::equal(held.columns(), renamed.get_schema()));
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address(), held_storage);
    auto nested_rename_data = rename_data;
    nested_rename_data.arg = renamed.get_index();
    nested_rename_data.columns[0] = x;
    query_data.variant = kr::dl::insert(repo, nested_rename_data).first.get_index();
    const auto nested = kr::dl::semantics::evaluate<tyr::GroundTag>(kr::dl::insert(repo, query_data).first, context);
    EXPECT_EQ(nested.columns()[0].label, ColumnIndex(ygg::uint_t(x)));
    EXPECT_EQ(nested.get_storage_address(), held_storage);
    EXPECT_EQ(nested.size(), held.size());
    EXPECT_EQ(held.columns()[0].label, ColumnIndex(ygg::uint_t(renamed_column)));
    // Owned labels and shared rows remain valid during repository/cache growth.
    for (int i = 0; i < 64; ++i)
    {
        column_data.name = "extra_" + std::to_string(i);
        const auto extra_column = kr::dl::insert(repo, column_data).first.get_index();
        rename_data.columns[0] = extra_column;
        query_data.variant = kr::dl::insert(repo, rename_data).first.get_index();
        const auto extra_query = kr::dl::insert(repo, query_data).first;
        const auto other = kr::dl::semantics::evaluate<tyr::GroundTag>(extra_query, context);
        EXPECT_EQ(other.get_storage_address(), held_storage);
        EXPECT_EQ(held.size(), 3);
        EXPECT_EQ(held.columns().data(), held_columns);
        EXPECT_EQ(held.columns()[0].label, ColumnIndex(ygg::uint_t(renamed_column)));
        EXPECT_EQ(other.columns()[0].label, ColumnIndex(ygg::uint_t(extra_column)));
    }

    // Repositories from one factory have distinct identities even when query indices match.
    auto& other_repo = *other_repository;
    EXPECT_NE(other_repo.get_index(), repo.get_index());
    column_data.name = "unused";
    static_cast<void>(kr::dl::insert(other_repo, column_data));
    column_data.name = "other";
    const auto other_column = kr::dl::insert(other_repo, column_data).first.get_index();
    top_wrapper.variant = kr::dl::insert(other_repo, top_data).first.get_index();
    lift_data.arg = kr::dl::insert(other_repo, top_wrapper).first.get_index();
    lift_data.columns[0] = other_column;
    query_data.variant = kr::dl::insert(other_repo, lift_data).first.get_index();
    const auto other_query = kr::dl::insert(other_repo, query_data).first;
    ASSERT_EQ(other_query.get_index(), query.get_index());
    ASSERT_NE(other_column, x);
    const auto cached = kr::dl::semantics::evaluate<tyr::GroundTag>(query, context);
    const auto other_cached = kr::dl::semantics::evaluate<tyr::GroundTag>(other_query, context);
    EXPECT_EQ(cached.columns()[0].label, ColumnIndex(ygg::uint_t(x)));
    EXPECT_EQ(other_cached.columns()[0].label, ColumnIndex(ygg::uint_t(other_column)));
    EXPECT_EQ(cached.size(), 3);
    EXPECT_EQ(other_cached.size(), 3);
    EXPECT_NE(cached, other_cached);
    EXPECT_EQ(cached.get_storage_address(), other_cached.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address(), cached.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(other_query, context).get_storage_address(), other_cached.get_storage_address());

    // Clear invalidates the borrowed views; reevaluation reconstructs rows and schemas.
    storage.reset_all();
    EXPECT_TRUE(caches.get_queries(true).empty());
    EXPECT_TRUE(caches.get_queries(false).empty());
    const auto rebuilt = kr::dl::semantics::evaluate<tyr::GroundTag>(renamed, context);
    EXPECT_EQ(rebuilt.size(), 3);
    EXPECT_EQ(rebuilt.columns()[0].label, ColumnIndex(ygg::uint_t(renamed_column)));
    EXPECT_EQ(rebuilt.get_storage_address(), kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address());
    storage.reset_all();
    repo.clear();
}

template<tyr::TaskKind Kind>
void check_query_cache_across_states()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(),
                                                                   builder,
                                                                   storage,
                                                                   kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                   kr::dl::semantics::insert(denotations, registers).first);
    const auto fixed = parse_query(R"((q_atomic_state "fixed" (x y z)))", domain, *repository);
    const auto goal = parse_query(R"((q_atomic_goal "triple" false (x y z)))", domain, *repository);
    const auto derived = parse_query(R"((q_atomic_state "copied" (x y z)))", domain, *repository);
    const auto ready = parse_query(R"((q_atomic_state "ready" ()))", domain, *repository);
    const auto triple = parse_query(R"((q_atomic_state "triple" (x y z)))", domain, *repository);
    const auto fixed_join = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto reversed_join = parse_query(R"((q_join (q_atomic_state "triple" (x y z)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto static_join = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto count = kr::ps::ext::dl::parse_numerical(R"((n_count (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto fixed_count = kr::ps::ext::dl::parse_numerical(R"((n_count (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto matching_row = std::tuple { domain.get_constants()[0].get_index().get_value(),
                                           domain.get_constants()[1].get_index().get_value(),
                                           domain.get_constants()[2].get_index().get_value() };
    const auto check_mixed_joins = [&](auto& target)
    {
        const auto matches = kr::dl::semantics::evaluate<Kind>(triple, target).contains(matching_row);
        for (const auto query : { fixed_join, reversed_join })
        {
            const auto rows = kr::dl::semantics::evaluate<Kind>(query, target);
            EXPECT_EQ(rows.size(), matches ? 1 : 0);
            EXPECT_EQ(rows.contains(matching_row), matches);
        }
        return matches;
    };

    const auto fixed_rows = kr::dl::semantics::evaluate<Kind>(fixed, context);
    const auto goal_rows = kr::dl::semantics::evaluate<Kind>(goal, context);
    EXPECT_EQ(fixed_rows.size(), 2);
    EXPECT_EQ(goal_rows.size(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(fixed, context).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(derived, context).size(), 4);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(ready, context).size(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(count, context).get(), 4);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(fixed_count, context).get(), 2);
    // A fully static join caches its result without retaining an additional index.
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(static_join, context).size(), fixed_rows.size());
    EXPECT_EQ(caches.get_static_join_indexes().size(), 0);
    EXPECT_TRUE(check_mixed_joins(context));
    // Opposite join orientations share one index over the same static rows and keys.
    EXPECT_EQ(caches.get_static_join_indexes().size(), 1);

    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    // Only removing (triple a b c) empties the join; the static index survives state changes.
    auto empty_joins = size_t { 0 };
    for (const auto& successor : successors)
    {
        auto successor_context =
            kr::dl::semantics::StateEvaluationContext<Kind>(successor.get_state(), builder, storage, context.arguments(), context.registers());
        storage.reset_dynamic();
        empty_joins += !check_mixed_joins(successor_context);
        EXPECT_EQ(caches.get_static_join_indexes().size(), 1);
    }
    EXPECT_EQ(empty_joins, 1);
    auto next = kr::dl::semantics::StateEvaluationContext<Kind>(successors.front().get_state(),
                                                                builder,
                                                                storage,
                                                                kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                kr::dl::semantics::insert(denotations, registers).first);
    storage.reset_dynamic();
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(false).empty());
    EXPECT_TRUE(caches.get_queries(true).contains(fixed));
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(true).contains(fixed_count));
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(fixed, next).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(goal, next).get_storage_address(), goal_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(derived, next).size(), 3);
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(ready, next).empty());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(count, next).get(), 3);

    const auto dynamic_role = kr::ps::ext::dl::parse_role(R"((r_project x z (q_atomic_state "triple" (x y z))))", domain, *repository);
    const auto static_role = kr::ps::ext::dl::parse_role(R"((r_atomic_state "edge"))", domain, *repository);
    const auto static_value = kr::dl::semantics::evaluate<Kind>(static_role, context);
    for (size_t i = 0; i < 16; ++i)
    {
        storage.reset_dynamic();
        EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
        EXPECT_EQ(static_value.count(), 2);
        const auto value = kr::dl::semantics::evaluate<Kind>(dynamic_role, i % 2 == 0 ? context : next);
        EXPECT_EQ(value.count(), i % 2 == 0 ? 4 : 3);
        EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 1);
        EXPECT_EQ(denotations.template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    }

    // Materialize persistent results directly; source and target scratch storage remain independent.
    storage.reset_dynamic();
    auto persistent_memo = kr::dl::semantics::DenotationCaches {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto retained = kr::dl::semantics::evaluate<Kind>(dynamic_role, persistent);
    EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    EXPECT_FALSE(caches.template get<kr::dl::RoleTag>(false).contains(dynamic_role));
    const auto source_value = kr::dl::semantics::evaluate<Kind>(dynamic_role, context);
    auto target_storage = kr::dl::semantics::EvaluationStorage(denotations);
    EXPECT_NE(&target_storage.get_denotation_repository(false), &storage.get_denotation_repository(false));
    EXPECT_NE(&storage.get_denotation_repository(false), &storage.get_denotation_repository(true));
    auto target_context =
        kr::dl::semantics::StateEvaluationContext<Kind>(successors.front().get_state(), builder, target_storage, next.arguments(), next.registers());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(dynamic_role, target_context).count(), 3);
    EXPECT_NE(kr::dl::semantics::evaluate<Kind>(static_role, target_context), static_value);
    target_storage.reset_all();
    EXPECT_EQ(source_value.count(), 4);
    EXPECT_EQ(static_value.count(), 2);
    EXPECT_EQ(retained.count(), 4);

    storage.reset_all();
    EXPECT_EQ(caches.get_static_join_indexes().size(), 0);
    EXPECT_TRUE(caches.get_queries(true).empty());
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(true).empty());
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(false).empty());
    EXPECT_EQ(storage.get_denotation_repository(false).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    EXPECT_EQ(storage.get_denotation_repository(true).template size<kr::dl::semantics::Denotation<kr::dl::RoleTag>>(), 0);
    EXPECT_EQ(retained.count(), 4);
    // A full clear releases indexed rows as well; reevaluation must rebuild the shared index.
    EXPECT_TRUE(check_mixed_joins(context));
    EXPECT_EQ(caches.get_static_join_indexes().size(), 1);
    storage.reset_all();
    EXPECT_TRUE(caches.get_queries(false).empty());
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(false).empty());
}

template<tyr::TaskKind Kind>
void check_query_cache_across_bindings()
{
    const auto search = query_search_context<Kind>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = kr::dl::semantics::StateEvaluationContext<Kind>(state,
                                                                   builder,
                                                                   storage,
                                                                   kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                   kr::dl::semantics::insert(denotations, registers).first);
    const auto a = domain.get_constants()[0];
    const auto b = domain.get_constants()[1];
    const auto c = domain.get_constants()[2];
    auto persistent_memo = kr::dl::semantics::DenotationCaches {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto a_set = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", domain, *repository), persistent);
    const auto b_set = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(R"((c_nominal "b"))", domain, *repository), persistent);
    auto argument_values = ygg::Data<kr::dl::semantics::CallArguments>();
    argument_values.concept_arguments.push_back(a_set.get_index());
    const auto make_bound = [&]
    {
        return kr::dl::semantics::StateEvaluationContext<Kind>(state,
                                                               builder,
                                                               storage,
                                                               kr::dl::semantics::insert(denotations, argument_values).first,
                                                               kr::dl::semantics::insert(denotations, registers).first);
    };
    registers.concept_values[0] = a.get_index();
    registers.role_values[0] = ::cista::pair(a.get_index(), b.get_index());
    auto bound = make_bound();
    const auto fixed = parse_query(R"((q_atomic_state "fixed" (x y z)))", domain, *repository);
    const auto register_count =
        kr::ps::ext::dl::parse_numerical(R"((n_count (q_join (q_atomic_state "fixed" (x y z)) (q_concept x (c_register 0)))))", domain, *repository);
    const auto argument = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_concept x (c_argument 0))))", domain, *repository);
    const auto role_register = parse_query(R"((q_join (q_atomic_state "fixed" (x y z)) (q_role (x y) (r_register 0))))", domain, *repository);
    const auto register_rhs = parse_query(R"((q_join (q_concept z (c_register 0)) (q_atomic_state "fixed" (x y z))))", domain, *repository);
    const auto role_register_rhs = parse_query(R"((q_join (q_role (x y) (r_register 0)) (q_atomic_state "fixed" (y x z))))", domain, *repository);

    const auto fixed_rows = kr::dl::semantics::evaluate<Kind>(fixed, bound);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(register_count, bound).get(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument, bound).size(), 1);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(role_register, bound).size(), 1);
    const auto rhs_rows = kr::dl::semantics::evaluate<Kind>(register_rhs, bound);
    EXPECT_EQ(rhs_rows.size(), 1);
    EXPECT_TRUE(rhs_rows.contains(std::tuple { a.get_index().get_value(), c.get_index().get_value(), b.get_index().get_value() }));
    EXPECT_TRUE(std::ranges::equal(rhs_rows.columns(), register_rhs.get_schema()));
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(role_register_rhs, bound).empty());
    registers.concept_values[0] = b.get_index();
    registers.role_values[0] = ::cista::pair(b.get_index(), c.get_index());
    argument_values.concept_arguments[0] = b_set.get_index();
    auto rebound = make_bound();
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(fixed, rebound).get_storage_address(), fixed_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(register_count, rebound).get(), 0);
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(argument, rebound).empty());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(role_register, rebound).empty());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(register_rhs, rebound).empty());
    // Reordered shared columns are matched by label, not by their position in either operand.
    const auto rhs_role_rows = kr::dl::semantics::evaluate<Kind>(role_register_rhs, rebound);
    EXPECT_EQ(rhs_role_rows.size(), 1);
    EXPECT_TRUE(rhs_role_rows.contains(std::tuple { b.get_index().get_value(), c.get_index().get_value(), a.get_index().get_value() }));
    EXPECT_TRUE(std::ranges::equal(rhs_role_rows.columns(), role_register_rhs.get_schema()));
    EXPECT_TRUE(caches.template get<kr::dl::NumericalTag>(false).contains(register_count));
    EXPECT_FALSE(caches.template get<kr::dl::NumericalTag>(true).contains(register_count));

    registers.concept_values[0].reset();
    registers.role_values[0].reset();
    argument_values.concept_arguments[0] = a_set.get_index();
    auto cleared = make_bound();
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(register_count, cleared).get(), 0);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument, cleared).size(), 1);
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(role_register, cleared).empty());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(register_rhs, cleared).empty());
    EXPECT_TRUE(kr::dl::semantics::evaluate<Kind>(role_register_rhs, cleared).empty());
    EXPECT_EQ(bound.registers().template get<kr::dl::ConceptTag>()[0].value().get_index(), a.get_index());
    EXPECT_EQ(bound.arguments().template get<kr::dl::ConceptTag>()[0].get_index(), a_set.get_index());
    EXPECT_EQ(rebound.registers().template get<kr::dl::ConceptTag>()[0].value().get_index(), b.get_index());
    EXPECT_EQ(rebound.arguments().template get<kr::dl::ConceptTag>()[0].get_index(), b_set.get_index());
}

template<tyr::TaskKind Kind>
void check_predicate_repository_identity()
{
    using Fact = tyr::formalism::FluentTag;
    using Predicate = tyr::formalism::Predicate<Fact>;
    const auto domain_source = std::string(R"(
(define (domain predicate-identity)
  (:requirements :strips)
  (:constants a b)
  (:predicates (flag) (marked ?x) (edge ?x ?y))
  (:action clear :parameters (?x ?y)
    :precondition (and (flag) (marked ?x) (edge ?x ?y))
    :effect (and (not (flag)) (not (marked ?x)) (not (edge ?x ?y)))))
)");
    const auto task_source = std::string(R"(
(define (problem predicate-identity-task)
  (:domain predicate-identity)
  (:init (flag) (marked a) (edge a b))
  (:goal (and (flag) (marked a) (edge a b))))
)");
    auto execution = ygg::ExecutionContext::create(1);
    auto lifted =
        tyr::planning::Task<tyr::LiftedTag>::create(tyr::formalism::planning::Parser(domain_source, std::nullopt).parse_task(task_source, std::nullopt));
    auto task = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return lifted->instantiate_ground_task(*execution).task;
        else
            return lifted;
    }();
    auto search = datasets::TaskSearchContext<Kind>::create(task, execution);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto& domain = task->get_domain();
    // Use one factory so repository identities differ even though predicate indices collide.
    const auto foreign = domain.get_repository_factory()->create_shared();
    auto predicates = std::array<ygg::Index<Predicate>, 3> {};
    ASSERT_EQ(domain.get_domain().template get_predicates<Fact>().size(), predicates.size());
    for (const auto predicate : domain.get_domain().template get_predicates<Fact>())
    {
        ASSERT_LT(predicate.get_arity(), predicates.size());
        predicates[predicate.get_arity()] = predicate.get_index();
        auto data = ygg::Data<Predicate>("foreign_" + predicate.get_name().str(), predicate.get_arity());
        const auto other = tyr::formalism::planning::insert(*foreign, data).first;
        ASSERT_EQ(other.get_index(), predicate.get_index());
        ASSERT_NE(other, predicate);
        EXPECT_EQ(ygg::make_view(predicate.get_index(), *task->get_repository()), predicate);
    }
    auto constructor_factory = kr::dl::ConstructorRepositoryFactory();
    for (const auto& planning_repository : { domain.get_repository(), task->get_repository(), foreign })
    {
        const auto expected = planning_repository != foreign;
        SCOPED_TRACE(expected ? "same predicate owner" : "different predicate owner");
        auto repository = constructor_factory.create(planning_repository);
        auto builder = kr::dl::semantics::Builder();
        auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(task->get_repository());
        auto storage = kr::dl::semantics::EvaluationStorage(denotations);
        auto context = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(), builder, storage);
        const auto check = [&]<typename Tag>()
        {
            auto concept_data = ygg::Data<kr::dl::Concept<Tag>>(predicates[1], true);
            auto concept_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::ConceptTag>>(kr::dl::insert(*repository, concept_data).first.get_index());
            EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(kr::dl::insert(*repository, concept_wrapper).first, context).get().count(), size_t(expected));

            auto role_data = ygg::Data<kr::dl::Role<Tag>>(predicates[2], true);
            auto role_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::RoleTag>>(kr::dl::insert(*repository, role_data).first.get_index());
            EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(kr::dl::insert(*repository, role_wrapper).first, context).count(), size_t(expected));

            auto boolean_data = ygg::Data<kr::dl::Boolean<Tag>>(predicates[0], true);
            auto boolean_wrapper = ygg::Data<kr::dl::Constructor<kr::dl::BooleanTag>>(kr::dl::insert(*repository, boolean_data).first.get_index());
            EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(kr::dl::insert(*repository, boolean_wrapper).first, context).get(), expected);

            auto column = ygg::Data<kr::dl::QueryColumn>();
            column.name = "x";
            auto query_data = ygg::Data<kr::dl::Query<Tag>>();
            query_data.predicate = predicates[1];
            query_data.columns.push_back(kr::dl::insert(*repository, column).first.get_index());
            if constexpr (kr::dl::is_atomic_goal_tag_v<Tag>)
                query_data.polarity = true;
            auto query_wrapper = ygg::Data<kr::dl::Query<>>();
            query_wrapper.variant = kr::dl::insert(*repository, query_data).first.get_index();
            EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(kr::dl::insert(*repository, query_wrapper).first, context).size(), size_t(expected));
        };
        check.template operator()<kr::dl::AtomicStateTag<Fact>>();
        check.template operator()<kr::dl::AtomicGoalTag<Fact>>();
    }
}

template<tyr::TaskKind Kind>
void check_query_object_views()
{
    const auto execution = ygg::ExecutionContext::create(1);
    auto lifted = tyr::planning::Task<tyr::LiftedTag>::create(tyr::formalism::planning::Parser(R"(
(define (domain query-objects)
  (:requirements :strips)
  (:constants parent)
  (:predicates (fixed ?x ?y)))
)",
                                                                                               "query-objects-domain.pddl")
                                                                  .parse_task(R"(
(define (problem query-objects-task)
  (:domain query-objects)
  (:objects local)
  (:init (fixed parent local))
  (:goal (fixed parent local)))
)",
                                                                              "query-objects-task.pddl"));
    const auto task = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return lifted->instantiate_ground_task(*execution).task;
        else
            return lifted;
    }();
    ASSERT_TRUE(task);
    const auto search = datasets::TaskSearchContext<Kind>::create(task, execution);
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    auto constructors = kr::dl::ConstructorRepositoryFactory().create(task->get_repository());
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(task->get_repository());
    auto builder = kr::dl::semantics::Builder {};
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments> {};
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues> {};
    auto context = kr::dl::semantics::StateEvaluationContext<Kind>(state,
                                                                   builder,
                                                                   storage,
                                                                   kr::dl::semantics::insert(denotations, arguments).first,
                                                                   kr::dl::semantics::insert(denotations, registers).first);
    const auto query = parse_query(R"((q_atomic_state "fixed" (x y)))", task->get_domain().get_domain(), *constructors);
    const auto constant = task->get_task().get_domain().get_constants()[0];
    const auto local = task->get_task().get_objects()[0];
    ASSERT_NE(&constant.get_context(), &local.get_context());
    const auto raw = std::vector { constant.get_index(), local.get_index() };
    const auto result = kr::dl::semantics::evaluate<Kind>(query, context);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(&result.get_context(), &storage.get_denotation_repository(true).get_relation_repository());
    // Rows hold raw object indices; the relation repository cannot resolve them.
    const auto row = result[0];
    static_assert(std::same_as<decltype(row.template get<ygg::uint_t>(size_t { 0 })), ygg::uint_t>);
    EXPECT_EQ(object_values(row), raw);
    EXPECT_TRUE(result.contains(std::tuple { constant.get_index().get_value(), local.get_index().get_value() }));

    storage.reset_dynamic();
    EXPECT_EQ(object_values(row), raw);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(query, context), result);
    storage.reset_all();
    const auto rebuilt = kr::dl::semantics::evaluate<Kind>(query, context);
    ASSERT_EQ(rebuilt.size(), 1);
    EXPECT_EQ(object_values(rebuilt[0]), raw);
}

template<tyr::TaskKind Kind>
void check_multiword_bit_evaluation()
{
    constexpr auto word_bits = static_cast<ygg::uint_t>(std::numeric_limits<ygg::uint_t>::digits);
    constexpr auto num_objects = 2 * word_bits + 3;
    constexpr auto last = num_objects - 1;
    const auto name = [](ygg::uint_t i) { return "v" + std::to_string(1000 + i); };
    auto domain_source = std::string("(define (domain bit-boundaries) (:requirements :strips) (:constants");
    for (ygg::uint_t i = 0; i < num_objects; ++i)
        domain_source += " " + name(i);
    domain_source += ") (:predicates (marked ?x) (edge ?x ?y)))";
    auto task_source = std::string("(define (problem bit-boundaries-task) (:domain bit-boundaries) (:init");
    for (const auto i : { static_cast<ygg::uint_t>(0), word_bits - 1, word_bits, last })
        task_source += " (marked " + name(i) + ")";
    for (const auto& [from, to] : std::array { std::pair { static_cast<ygg::uint_t>(0), word_bits - 1 },
                                               std::pair { static_cast<ygg::uint_t>(0), word_bits },
                                               std::pair { word_bits - 1, last },
                                               std::pair { word_bits, last } })
        task_source += " (edge " + name(from) + " " + name(to) + ")";
    task_source += ") (:goal (and)))";
    auto execution = ygg::ExecutionContext::create(1);
    auto lifted =
        tyr::planning::Task<tyr::LiftedTag>::create(tyr::formalism::planning::Parser(domain_source, std::nullopt).parse_task(task_source, std::nullopt));
    auto task = [&]
    {
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            return lifted->instantiate_ground_task(*execution).task;
        else
            return lifted;
    }();
    auto search = datasets::TaskSearchContext<Kind>::create(task, execution);
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = task->get_domain().get_domain();
    ASSERT_EQ(domain.get_constants().size(), num_objects);
    for (const auto i : { static_cast<ygg::uint_t>(0), word_bits - 1, word_bits, last })
    {
        ASSERT_EQ(domain.get_constants()[i].get_name(), name(i));
        ASSERT_EQ(ygg::uint_t(domain.get_constants()[i].get_index()), i);
    }
    auto repository = kr::dl::ConstructorRepositoryFactory().create(task->get_repository());
    kr::dl::semantics::Builder builder;
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(task->get_repository());
    kr::dl::semantics::EvaluationStorage storage(denotations);
    ygg::Data<kr::dl::semantics::CallArguments> arguments;
    ygg::Data<kr::dl::semantics::RegisterValues> registers;
    auto context = kr::dl::semantics::StateEvaluationContext<Kind>(initial.get_state(),
                                                                   builder,
                                                                   storage,
                                                                   kr::dl::semantics::insert(denotations, arguments).first,
                                                                   kr::dl::semantics::insert(denotations, registers).first);
    const auto query = [&](const std::string& expression) { return kr::dl::semantics::evaluate<Kind>(parse_query(expression, domain, *repository), context); };
    const auto concept_value = [&](const std::string& expression)
    { return kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(expression, domain, *repository), context); };
    const auto number = [&](const std::string& expression)
    { return kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_numerical(expression, domain, *repository), context).get(); };
    const auto marked = std::string(R"((c_atomic_state "marked"))");
    const auto edge = std::string(R"((r_atomic_state "edge"))");

    const auto selected = query("(q_concept x " + marked + ")");
    ASSERT_EQ(selected.size(), 4);
    for (const auto i : { static_cast<ygg::uint_t>(0), word_bits - 1, word_bits, last })
        EXPECT_TRUE(selected.contains(std::tuple { ygg::uint_t(i) }));
    EXPECT_EQ(query("(q_concept x (c_top))").size(), num_objects);
    EXPECT_TRUE(query("(q_concept x (c_bot))").empty());
    const auto identity = query("(q_role (x y) (r_identity (c_top)))");
    EXPECT_EQ(identity.size(), num_objects);
    EXPECT_TRUE(identity.contains(std::tuple { ygg::uint_t(last), ygg::uint_t(last) }));
    EXPECT_TRUE(query("(q_role (x y) (r_identity (c_bot)))").empty());

    const auto inverse = query("(q_role (x y) (r_inverse " + edge + "))");
    ASSERT_EQ(inverse.size(), 4);
    EXPECT_TRUE(inverse.contains(std::tuple { ygg::uint_t(word_bits - 1), ygg::uint_t(0) }));
    EXPECT_TRUE(inverse.contains(std::tuple { ygg::uint_t(word_bits), ygg::uint_t(0) }));
    EXPECT_TRUE(inverse.contains(std::tuple { ygg::uint_t(last), ygg::uint_t(word_bits - 1) }));
    EXPECT_TRUE(inverse.contains(std::tuple { ygg::uint_t(last), ygg::uint_t(word_bits) }));
    const auto composition = query("(q_role (x y) (r_composition " + edge + " " + edge + "))");
    ASSERT_EQ(composition.size(), 1);
    EXPECT_TRUE(composition.contains(std::tuple { ygg::uint_t(0), ygg::uint_t(last) }));
    EXPECT_TRUE(query("(q_role (x y) (r_composition " + edge + " (r_identity (c_bot))))").empty());

    const auto at_least = concept_value("(c_at_least 2 " + edge + " " + marked + ")").get();
    EXPECT_EQ(at_least.count(), 1);
    EXPECT_TRUE(at_least.test(0));
    const auto exactly = concept_value("(c_exactly 1 " + edge + " " + marked + ")").get();
    EXPECT_EQ(exactly.count(), 2);
    EXPECT_TRUE(exactly.test(word_bits - 1));
    EXPECT_TRUE(exactly.test(word_bits));
    const auto at_most = concept_value("(c_at_most 1 " + edge + " " + marked + ")").get();
    EXPECT_EQ(at_most.count(), num_objects - 1);
    EXPECT_FALSE(at_most.test(0));
    EXPECT_EQ(concept_value("(c_exactly 0 " + edge + " (c_bot))").get().count(), num_objects);

    // The second source crosses a block boundary and shortens the path from two steps to one.
    const auto starts = "(c_or (c_nominal \"" + name(0) + "\") (c_nominal \"" + name(word_bits) + "\"))";
    EXPECT_EQ(number("(n_distance " + starts + " " + edge + " (c_nominal \"" + name(last) + "\"))"), 1);
    EXPECT_EQ(number("(n_distance (c_nominal \"" + name(last) + "\") " + edge + " (c_nominal \"" + name(0) + "\"))"), std::numeric_limits<ygg::uint_t>::max());
}

}  // namespace

TEST(RunirQueries, GroundQueryRowsResolveCanonicalObjectViews) { check_query_object_views<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedQueryRowsResolveCanonicalObjectViews) { check_query_object_views<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundMultiwordBitEvaluation) { check_multiword_bit_evaluation<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedMultiwordBitEvaluation) { check_multiword_bit_evaluation<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundAtomicEvaluationChecksPredicateRepository) { check_predicate_repository_identity<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedAtomicEvaluationChecksPredicateRepository) { check_predicate_repository_identity<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundRelationsAndMutableBindings) { check_queries<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedRelationsAndMutableBindings) { check_queries<tyr::LiftedTag>(); }

namespace
{
/// The previous evaluation: min_distance over fully evaluated sources, edges, and targets.
template<tyr::TaskKind Kind, typename Context>
ygg::uint_t reference_distance(kr::dl::ConstructorView<kr::dl::NumericalTag> numerical, Context& context)
{
    return ygg::visit(
        [&]<typename Concrete>(Concrete concrete) -> ygg::uint_t
        {
            if constexpr (requires { concrete.get_mid(); })
            {
                const auto& plan = concrete.get_data().plan;
                auto children = context.child_context();
                const auto relation = [&](auto argument, auto columns)
                {
                    return ygg::visit([&](auto child) { return kr::dl::semantics::detail::evaluate_relation<Kind>(child, columns.span(), children); },
                                      argument);
                };
                const auto sources = relation(concrete.get_lhs(), plan.source_columns());
                const auto edges = relation(concrete.get_mid(), plan.edge_columns());
                const auto targets = relation(concrete.get_rhs(), plan.target_columns());
                auto distances = ygg::Builder<ygg::database::Relation>(plan.output_columns().span());
                auto workspace = ygg::database::DistanceWorkspace(plan);
                return ygg::database::min_distance(sources, edges, targets, plan, distances, workspace);
            }
            else
                throw std::logic_error("reference_distance expects n_distance");
        },
        numerical.get_variant());
}

/// Reuse across evaluations (the path tree) must stay exact while edges disappear and reappear between states:
/// evaluations alternate between the initial state and each successor, sharing one builder.
template<tyr::TaskKind Kind>
void check_distance_reuse_across_states()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto successors = search->successor_generator->get_labeled_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    const auto argument_view = kr::dl::semantics::insert(denotations, arguments).first;
    const auto register_view = kr::dl::semantics::insert(denotations, registers).first;

    const auto edge = std::string(R"((r_atomic_state "edge"))");
    const auto edges = "(q_role (x y) " + edge + ")";
    const auto pair_edges = "(q_project (x u y v) (q_join " + edges + " (q_role (u v) " + edge + ")))";
    const auto pair = [](const char* name) { return "(q_role (x u) (r_identity (c_nominal \"" + std::string(name) + "\")))"; };
    // `triple` is dynamic: removing a triple removes edges, and returning to the initial state adds them again.
    const auto with_triple = [&](const char* middle)
    {
        return "(q_project (x u y v) (q_join " + pair_edges + " (q_project (x) (q_select_value m \"" + std::string(middle)
               + "\" (q_atomic_state \"triple\" (x m n))))))";
    };
    const auto expressions = std::vector<std::string> {
        "(n_distance " + pair("a") + " " + with_triple("b") + " " + pair("c") + ")",
        "(n_distance " + pair("b") + " " + with_triple("c") + " " + pair("c") + ")",
        "(n_distance " + pair("a") + " (q_union " + with_triple("b") + " " + with_triple("c") + ") " + pair("c") + ")",
    };
    auto numericals = std::vector<kr::dl::ConstructorView<kr::dl::NumericalTag>>();
    for (const auto& expression : expressions)
        numericals.push_back(kr::ps::ext::dl::parse_numerical(expression, domain, *repository));

    auto states = std::vector<tyr::planning::Node<Kind>> { initial };
    for (const auto& successor : successors)
    {
        states.push_back(successor.node);
        states.push_back(initial);
    }
    for (size_t round = 0; round < 2; ++round)
        for (const auto& node : states)
        {
            storage.reset_dynamic();
            auto context = kr::dl::semantics::StateEvaluationContext<Kind>(node.get_state(), builder, storage, argument_view, register_view);
            for (size_t i = 0; i < numericals.size(); ++i)
                EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(numericals[i], context).get(), reference_distance<Kind>(numericals[i], context)) << expressions[i];
        }
}

}  // namespace

TEST(RunirQueries, GroundDistanceOverQueries) { check_distance_queries<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedDistanceOverQueries) { check_distance_queries<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundDistanceReuseAcrossStates) { check_distance_reuse_across_states<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedDistanceReuseAcrossStates) { check_distance_reuse_across_states<tyr::LiftedTag>(); }
TEST(RunirQueries, CachingAndBorrowedRenameLifetime) { check_cached_queries(); }
TEST(RunirQueries, GroundQueryCachePreservesStaticResultsAcrossStates) { check_query_cache_across_states<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedQueryCachePreservesStaticResultsAcrossStates) { check_query_cache_across_states<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundQueryCacheInvalidatesRegistersAndArguments) { check_query_cache_across_bindings<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedQueryCacheInvalidatesRegistersAndArguments) { check_query_cache_across_bindings<tyr::LiftedTag>(); }

TEST(RunirQueries, StaticnessIsInferredAndSerializedWithoutChangingIdentity)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    const auto fixed = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto triple = std::string(R"((q_atomic_state "triple" (x y z)))");
    const auto cases = std::vector<std::pair<std::string, bool>> {
        { fixed, true },
        { triple, false },
        { R"((q_atomic_state "copied" (x y z)))", false },
        { R"((q_atomic_goal "triple" true (x y z)))", true },
        { R"((q_atomic_goal "triple" false (x y z)))", true },
        { "(q_join " + fixed + " " + fixed + ")", true },
        { "(q_join " + fixed + " " + triple + ")", false },
        { "(q_project () " + fixed + ")", true },
        { "(q_rename (a b c) " + triple + ")", false },
        { "(q_select_equal y z " + fixed + ")", true },
        { "(q_select_value x \"a\" " + triple + ")", false },
        { "(q_union " + fixed + " " + fixed + ")", true },
        { "(q_difference " + fixed + " " + triple + ")", false },
        { R"((q_concept x (c_top)))", true },
        { R"((q_concept x (c_register 0)))", false },
        { R"((q_concept x (c_argument 0)))", false },
        { R"((q_role (x y) (r_atomic_state "edge")))", true },
        { R"((q_role (x y) (r_register 0)))", false },
        { R"((q_role (x y) (r_argument 0)))", false },
    };
    const auto check_metadata = [&](auto expression, bool expected)
    {
        EXPECT_EQ(expression.is_static(), expected);
        check_relocated_data(expression.get_data(), [&](const auto& decoded) { EXPECT_EQ(decoded.is_static, expected); });
        auto stale = expression.get_data();
        stale.is_static = !expected;
        EXPECT_EQ(stale, expression.get_data());
        using Data = std::remove_cvref_t<decltype(stale)>;
        EXPECT_EQ(ygg::Hash<Data>()(stale), ygg::Hash<Data>()(expression.get_data()));
        const auto [same, created] = ygg::formalism::insert(*repository, stale);
        EXPECT_FALSE(created);
        EXPECT_EQ(same.get_index(), expression.get_index());
        EXPECT_EQ(stale.is_static, expected);
        stale.clear();
        EXPECT_FALSE(stale.is_static);
    };
    for (const auto& [description, expected] : cases)
    {
        SCOPED_TRACE(description);
        check_metadata(parse_query(description, domain, *repository), expected);
        check_metadata(kr::ps::ext::dl::parse_numerical("(n_count " + description + ")", domain, *repository), expected);
    }
    EXPECT_TRUE(kr::ps::ext::dl::parse_concept("(c_project x " + fixed + ")", domain, *repository).is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_concept("(c_project x " + triple + ")", domain, *repository).is_static());
    EXPECT_TRUE(kr::ps::ext::dl::parse_role(R"((r_reflexive_transitive_closure (r_atomic_state "edge")))", domain, *repository).is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_role(R"((r_reflexive_transitive_closure (r_register 0)))", domain, *repository).is_static());
    EXPECT_TRUE(kr::ps::ext::dl::parse_boolean("(b_nonempty " + fixed + ")", domain, *repository).is_static());
    EXPECT_TRUE(kr::ps::ext::dl::parse_boolean("(b_nonempty (q_generic_join (x y z) (x y z) " + fixed + " " + fixed + "))", domain, *repository).is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_boolean("(b_nonempty (q_generic_join (x y z) (x y z) " + fixed + " " + triple + "))", domain, *repository).is_static());
    EXPECT_TRUE(kr::ps::ext::dl::parse_boolean("(b_nonempty (q_empty (x)))", domain, *repository).is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_boolean("(b_nonempty (q_must_pass (x) (q_project (x y) " + triple + ") " + triple + " (q_project (x z) " + triple
                                                    + R"() (q_concept p (c_top)))))",
                                                domain,
                                                *repository)
                     .is_static());
    EXPECT_FALSE(kr::ps::ext::dl::parse_boolean("(b_nonempty " + triple + ")", domain, *repository).is_static());
}

TEST(RunirQueries, CachedRenameSharesRowsWithoutChangingSchemasAndHandlesNullaryJoins)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage(denotations);
    auto& caches = storage.get_caches();
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(state,
                                                                             builder,
                                                                             storage,
                                                                             kr::dl::semantics::insert(denotations, empty_arguments).first,
                                                                             kr::dl::semantics::insert(denotations, registers).first);
    const auto description = std::string(R"((q_atomic_state "fixed" (x y z)))");
    const auto query = parse_query(description, domain, *repository);
    const auto renamed = parse_query("(q_rename (u v w) " + description + ")", domain, *repository);
    const auto nested = parse_query("(q_rename (p q r) (q_rename (u v w) " + description + "))", domain, *repository);
    const auto original_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(query, context);
    const auto renamed_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(renamed, context);
    const auto nested_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(nested, context);
    EXPECT_EQ(renamed_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_EQ(nested_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(renamed, context).get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(std::ranges::equal(original_rows.columns(), query.get_schema()));
    EXPECT_TRUE(std::ranges::equal(renamed_rows.columns(), renamed.get_schema()));
    EXPECT_TRUE(std::ranges::equal(nested_rows.columns(), nested.get_schema()));
    EXPECT_FALSE(std::ranges::equal(original_rows.columns(), renamed_rows.columns()));

    const auto truth = parse_query("(q_project () " + description + ")", domain, *repository);
    const auto true_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(truth, context);
    EXPECT_TRUE(true_rows.columns().empty());
    EXPECT_EQ(true_rows.size(), 1);
    const auto true_join = parse_query("(q_join " + description + " (q_project () " + description + "))", domain, *repository);
    const auto false_join = parse_query("(q_join " + description + " (q_atomic_state \"missing\" ()))", domain, *repository);
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(true_join, context).size(), original_rows.size());
    EXPECT_TRUE(kr::dl::semantics::evaluate<tyr::GroundTag>(false_join, context).empty());
    EXPECT_TRUE(std::ranges::equal(kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).columns(), query.get_schema()));

    auto raw_rename_data = ygg::Data<kr::dl::Query<kr::dl::QueryRenameTag>>();
    raw_rename_data.arg = query.get_index();
    for (const auto column : query.get_columns())
        raw_rename_data.columns.push_back(column.get_index());
    std::swap(raw_rename_data.columns.front(), raw_rename_data.columns.back());
    auto raw_wrapper = ygg::Data<kr::dl::Query<>>(kr::dl::insert(*repository, raw_rename_data).first.get_index());
    const auto raw = repository->insert(raw_wrapper).first;
    EXPECT_FALSE(raw.is_static());
    const auto raw_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(raw, context);
    EXPECT_EQ(raw_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw));

    auto raw_nested_data = ygg::Data<kr::dl::Query<kr::dl::QueryRenameTag>>();
    raw_nested_data.arg = raw.get_index();
    for (const auto column : nested.get_columns())
        raw_nested_data.columns.push_back(column.get_index());
    auto raw_nested_wrapper = ygg::Data<kr::dl::Query<>>(kr::dl::insert(*repository, raw_nested_data).first.get_index());
    const auto raw_nested = repository->insert(raw_nested_wrapper).first;
    EXPECT_FALSE(raw_nested.is_static());
    const auto raw_nested_rows = kr::dl::semantics::evaluate<tyr::GroundTag>(raw_nested, context);
    EXPECT_EQ(raw_nested_rows.get_storage_address(), original_rows.get_storage_address());
    EXPECT_TRUE(std::ranges::equal(raw_nested_rows.columns(), raw_nested.get_schema()));
    EXPECT_TRUE(std::ranges::equal(raw_rows.columns(), raw.get_schema()));
    EXPECT_TRUE(std::ranges::equal(original_rows.columns(), query.get_schema()));
    EXPECT_TRUE(caches.get_queries(false).contains(raw_nested));

    storage.reset_all();
    EXPECT_FALSE(caches.get_queries(false).contains(raw));
    EXPECT_FALSE(caches.get_queries(false).contains(raw_nested));
    EXPECT_FALSE(caches.get_queries(true).contains(query));
    const auto rebuilt_nested = kr::dl::semantics::evaluate<tyr::GroundTag>(raw_nested, context);
    EXPECT_EQ(rebuilt_nested.size(), 2);
    EXPECT_TRUE(std::ranges::equal(rebuilt_nested.columns(), raw_nested.get_schema()));
    EXPECT_EQ(rebuilt_nested.get_storage_address(), kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw_nested));
    const auto rebuilt = kr::dl::semantics::evaluate<tyr::GroundTag>(raw, context);
    EXPECT_EQ(rebuilt.size(), 2);
    EXPECT_TRUE(std::ranges::equal(rebuilt.columns(), raw.get_schema()));
    EXPECT_EQ(rebuilt.get_storage_address(), kr::dl::semantics::evaluate<tyr::GroundTag>(query, context).get_storage_address());
    EXPECT_TRUE(caches.get_queries(false).contains(raw));
}

TEST(RunirQueries, QueryResultIdentityIncludesOrderedSchemaAndUnorderedRows)
{
    const auto search = query_search_context<tyr::GroundTag>();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder {};
    auto& repository = denotations.get_relation_repository();
    using Row = std::array<ObjectIndex, 2>;
    const auto intern = [&](std::array<ColumnIndex, 2> columns, std::array<Row, 2> rows)
    {
        auto result = builder.get_builder<ygg::database::Relation>(columns);
        for (const auto& row : rows)
            result->insert(std::tuple { row[0].get_value(), row[1].get_value() });
        return ygg::database::insert(repository, *result).first;
    };
    const auto columns = std::array<ColumnIndex, 2> { ColumnIndex(0), ColumnIndex(1) };
    const auto rows = std::array<Row, 2> { Row { ObjectIndex(2), ObjectIndex(3) }, Row { ObjectIndex(4), ObjectIndex(5) } };
    const auto first = intern(columns, rows);
    EXPECT_EQ(intern(columns, rows), first);
    EXPECT_NE(intern({ ColumnIndex(1), ColumnIndex(0) }, rows), first);
    EXPECT_EQ(intern(columns, { rows[1], rows[0] }), first);
    EXPECT_EQ(repository.size(), 2);

    auto renamed_columns = std::array<ColumnIndex, 2> { ColumnIndex(6), ColumnIndex(7) };
    const auto renamed = repository.rename(first, renamed_columns);
    renamed_columns[0] = ColumnIndex(99);
    EXPECT_EQ(renamed.columns()[0].label, ColumnIndex(6));
    EXPECT_EQ(renamed.get_storage_address(), first.get_storage_address());
    EXPECT_NE(renamed, first);
    EXPECT_EQ(repository.rename(renamed, columns), first);
}

TEST(RunirQueries, PersistentRenameSurvivesIntermediateResetAndConstructorRelease)
{
    const auto search = query_search_context<tyr::GroundTag>();
    const auto state = search->state_repository->get_initial_state(*search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto constructors = kr::dl::ConstructorRepositoryFactory().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder {};
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto intermediates = kr::dl::semantics::EvaluationStorage(denotations);
    auto output_memo = kr::dl::semantics::DenotationCaches {};
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments> {};
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues> {};
    auto context = kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>(state,
                                                                             builder,
                                                                             output_memo,
                                                                             denotations,
                                                                             intermediates,
                                                                             kr::dl::semantics::insert(denotations, arguments).first,
                                                                             kr::dl::semantics::insert(denotations, registers).first);
    const auto expression = parse_query(R"((q_rename (a b c) (q_atomic_state "triple" (x y z))))", domain, *constructors);
    const auto result = kr::dl::semantics::evaluate<tyr::GroundTag>(expression, context);
    ASSERT_EQ(result.size(), 4);
    const auto schema = column_labels(result.columns());
    const auto first = object_values(result[0]);
    intermediates.reset_all();
    EXPECT_EQ(kr::dl::semantics::evaluate<tyr::GroundTag>(expression, context), result);
    output_memo.reset_all();
    constructors->clear();
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(column_labels(result.columns()), schema);
    EXPECT_EQ(object_values(result[0]), first);
}

}  // namespace runir::tests
