#include "query_fixtures.hpp"

namespace runir::tests
{
namespace
{

/// Distance arguments may be concepts/roles or queries; every form shares one evaluation.
template<tyr::TaskKind Kind>
void check_distance_queries()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(initial.get_state(),
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
                    { return ygg::visit([&](auto child) { return kr::dl::semantics::detail::evaluate_relation<Kind>(child, columns.span(), children); }, argument); };
                    const auto sources = relation(concrete.get_lhs(), plan.source_columns());
                    const auto edges = relation(concrete.get_mid(), plan.edge_columns());
                    const auto targets = relation(concrete.get_rhs(), plan.target_columns());
                    auto distances = ygg::Builder<ygg::database::Relation<kr::dl::QueryValues>>(plan.output_columns().span());
                    auto workspace = ygg::database::DistanceWorkspace<kr::dl::QueryValues>(plan);
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
    EXPECT_EQ(number("(n_distance " + vertex("a") + " (q_difference " + edges + " (q_join (q_concept x (c_nominal \"a\")) (q_concept y (c_nominal \"b\")))) " + vertex("c") + ")"), infinity);
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
    const auto walk = "(q_project (x u y v) (q_join (q_join " + edges + " (q_role (u v) (r_identity (c_top)))) (q_role (y u) (r_complement (r_identity (c_top))))))";
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
    { return "(q_project (x u y v) (q_join " + pair_edges + " (q_project (x) (q_select_value m \"" + std::string(middle) + "\" (q_atomic_state \"triple\" (x m n))))))"; };
    EXPECT_EQ(number("(n_distance " + pair("a") + " " + with_triple("b") + " " + pair("c") + ")"), 2);
    EXPECT_EQ(number("(n_distance " + pair("a") + " " + with_triple("c") + " " + pair("c") + ")"), infinity);
    EXPECT_EQ(number("(n_distance " + pair("b") + " " + with_triple("c") + " " + pair("c") + ")"), 1);

    // Insertion requires arities k, 2k, k.
    using Distance = ygg::Data<kr::dl::Numerical<Ext, kr::dl::DistanceTag>>;
    const auto source = kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", domain, *repository);
    const auto role = kr::ps::ext::dl::parse_role(edge, domain, *repository);
    auto mismatched = Distance(source.get_index(), role.get_index(), parse_query(pair("c"), domain, *repository).get_index());
    EXPECT_THROW(static_cast<void>(kr::dl::insert(*repository, mismatched)), std::invalid_argument);
    mismatched = Distance(source.get_index(), parse_query(pair_edges, domain, *repository).get_index(), source.get_index());
    EXPECT_THROW(static_cast<void>(kr::dl::insert(*repository, mismatched)), std::invalid_argument);
}

}  // namespace

/// The previous evaluation: min_distance over fully evaluated sources, edges, and targets.
template<tyr::TaskKind Kind, typename Context>
ygg::uint_t reference_distance(kr::dl::FamilyConstructorView<Ext, kr::dl::NumericalTag> numerical, Context& context)
{
    return ygg::visit(
        [&]<typename Concrete>(Concrete concrete) -> ygg::uint_t
        {
            if constexpr (requires { concrete.get_mid(); })
            {
                const auto& plan = concrete.get_data().plan;
                auto children = context.child_context();
                const auto relation = [&](auto argument, auto columns)
                { return ygg::visit([&](auto child) { return kr::dl::semantics::detail::evaluate_relation<Kind>(child, columns.span(), children); }, argument); };
                const auto sources = relation(concrete.get_lhs(), plan.source_columns());
                const auto edges = relation(concrete.get_mid(), plan.edge_columns());
                const auto targets = relation(concrete.get_rhs(), plan.target_columns());
                auto distances = ygg::Builder<ygg::database::Relation<kr::dl::QueryValues>>(plan.output_columns().span());
                auto workspace = ygg::database::DistanceWorkspace<kr::dl::QueryValues>(plan);
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
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
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
    { return "(q_project (x u y v) (q_join " + pair_edges + " (q_project (x) (q_select_value m \"" + std::string(middle) + "\" (q_atomic_state \"triple\" (x m n))))))"; };
    const auto expressions = std::vector<std::string> {
        "(n_distance " + pair("a") + " " + with_triple("b") + " " + pair("c") + ")",
        "(n_distance " + pair("b") + " " + with_triple("c") + " " + pair("c") + ")",
        "(n_distance " + pair("a") + " (q_union " + with_triple("b") + " " + with_triple("c") + ") " + pair("c") + ")",
    };
    auto numericals = std::vector<kr::dl::FamilyConstructorView<Ext, kr::dl::NumericalTag>>();
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
            auto context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(node.get_state(), builder, storage, argument_view, register_view);
            for (size_t i = 0; i < numericals.size(); ++i)
                EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(numericals[i], context).get(), reference_distance<Kind>(numericals[i], context)) << expressions[i];
        }
}

TEST(RunirQueries, GroundDistanceOverQueries) { check_distance_queries<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedDistanceOverQueries) { check_distance_queries<tyr::LiftedTag>(); }
TEST(RunirQueries, GroundDistanceReuseAcrossStates) { check_distance_reuse_across_states<tyr::GroundTag>(); }
TEST(RunirQueries, LiftedDistanceReuseAcrossStates) { check_distance_reuse_across_states<tyr::LiftedTag>(); }

}  // namespace runir::tests
