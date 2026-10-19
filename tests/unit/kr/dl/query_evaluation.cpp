#include "query_fixtures.hpp"

namespace runir::tests
{
namespace
{

template<tyr::TaskKind Kind>
void check_queries()
{
    const auto search = query_search_context<Kind>();
    const auto initial = search->successor_generator->get_initial_node(*search->state_repository, *search->axiom_evaluator);
    const auto domain = search->task->get_domain().get_domain();
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(search->task->get_repository());
    auto builder = kr::dl::semantics::Builder();
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(search->task->get_repository());
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
    auto empty_arguments = ygg::Data<kr::dl::semantics::CallArguments>();
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues>();
    registers.concept_values.resize(1);
    registers.role_values.resize(1);
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(initial.get_state(),
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
        auto updated = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(initial.get_state(),
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

    auto persistent_memo = kr::dl::semantics::DenotationCaches<Ext> {};
    auto persistent = persistent_context(context, persistent_memo, denotations, storage);
    const auto a_set = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(R"((c_nominal "a"))", domain, *repository), persistent);
    const auto c_set = kr::dl::semantics::evaluate<Kind>(kr::ps::ext::dl::parse_concept(R"((c_nominal "c"))", domain, *repository), persistent);
    auto argument_values = ygg::Data<kr::dl::semantics::CallArguments>();
    argument_values.concept_arguments.push_back(a_set.get_index());
    auto argument_context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                                                 builder,
                                                                                 storage,
                                                                                 kr::dl::semantics::insert(denotations, argument_values).first,
                                                                                 kr::dl::semantics::insert(denotations, registers).first);
    const auto argument_count = kr::ps::ext::dl::parse_numerical("(n_count (q_join " + triple + " (q_concept x (c_argument 0))))", domain, *repository);
    // An invalid Ext argument must propagate through DL/query variant dispatch, not terminate.
    EXPECT_THROW(kr::dl::semantics::evaluate<Kind>(argument_count, context), std::out_of_range);
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument_count, argument_context).get(), 2);
    const auto argument = kr::ps::ext::dl::parse_concept("(c_argument 0)", domain, *repository);
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument, argument_context), a_set);
    argument_values.concept_arguments[0] = c_set.get_index();
    auto other_argument_context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(initial.get_state(),
                                                                                       builder,
                                                                                       storage,
                                                                                       kr::dl::semantics::insert(denotations, argument_values).first,
                                                                                       kr::dl::semantics::insert(denotations, registers).first);
    storage.reset_dynamic();
    EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(argument_count, other_argument_context).get(), 0);
    EXPECT_EQ(argument_context.arguments().template get<kr::dl::ConceptTag>()[0].get_index(), a_set.get_index());

    const auto successors = search->successor_generator->get_successor_nodes(initial, *search->state_repository, *search->axiom_evaluator);
    ASSERT_FALSE(successors.empty());
    auto next = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(successors.front().get_state(),
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
        const auto planning_state = initial.get_state();
        auto borrowed_context =
            kr::dl::semantics::StateEvaluationContext<Ext, Kind, tyr::planning::BuilderStateView<Kind>, kr::dl::semantics::BorrowedRegisterValuesView>(
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
    auto destination_memo = kr::dl::semantics::DenotationCaches<Ext> {};
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
    denotations.clear();
    EXPECT_EQ(retained_distance.get(), 2);
}

template<tyr::TaskKind Kind>
void check_predicate_repository_identity()
{
    using Family = kr::BaseFamilyTag;
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
    auto constructor_factory = kr::dl::ConstructorRepositoryFactoryFor<Family>();
    for (const auto& planning_repository : { domain.get_repository(), task->get_repository(), foreign })
    {
        const auto expected = planning_repository != foreign;
        SCOPED_TRACE(expected ? "same predicate owner" : "different predicate owner");
        auto repository = constructor_factory.create(planning_repository);
        auto builder = kr::dl::semantics::Builder();
        auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(task->get_repository());
        auto storage = kr::dl::semantics::EvaluationStorage<Family>(denotations);
        auto context = kr::dl::semantics::StateEvaluationContext<Family, Kind>(initial.get_state(), builder, storage);
        const auto check = [&]<typename Tag>()
        {
            auto concept_data = ygg::Data<kr::dl::Concept<Family, Tag>>(predicates[1], true);
            auto concept_wrapper = ygg::Data<kr::dl::Constructor<Family, kr::dl::ConceptTag>>(kr::dl::insert(*repository, concept_data).first.get_index());
            EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(kr::dl::insert(*repository, concept_wrapper).first, context).get().count(), size_t(expected));

            auto role_data = ygg::Data<kr::dl::Role<Family, Tag>>(predicates[2], true);
            auto role_wrapper = ygg::Data<kr::dl::Constructor<Family, kr::dl::RoleTag>>(kr::dl::insert(*repository, role_data).first.get_index());
            EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(kr::dl::insert(*repository, role_wrapper).first, context).count(), size_t(expected));

            auto boolean_data = ygg::Data<kr::dl::Boolean<Family, Tag>>(predicates[0], true);
            auto boolean_wrapper = ygg::Data<kr::dl::Constructor<Family, kr::dl::BooleanTag>>(kr::dl::insert(*repository, boolean_data).first.get_index());
            EXPECT_EQ(kr::dl::semantics::evaluate<Kind>(kr::dl::insert(*repository, boolean_wrapper).first, context).get(), expected);

            auto column = ygg::Data<kr::dl::QueryColumn>();
            column.name = "x";
            auto query_data = ygg::Data<kr::dl::Query<Family, Tag>>();
            query_data.predicate = predicates[1];
            query_data.columns.push_back(kr::dl::insert(*repository, column).first.get_index());
            if constexpr (kr::dl::is_atomic_goal_tag_v<Tag>)
                query_data.polarity = true;
            auto query_wrapper = ygg::Data<kr::dl::Query<Family>>();
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
    const auto lifted = tyr::planning::Task<tyr::LiftedTag>::create(tyr::formalism::planning::Parser(R"(
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
    auto constructors = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(task->get_repository());
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(task->get_repository());
    auto builder = kr::dl::semantics::Builder {};
    auto storage = kr::dl::semantics::EvaluationStorage<Ext>(denotations);
    auto arguments = ygg::Data<kr::dl::semantics::CallArguments> {};
    auto registers = ygg::Data<kr::dl::semantics::RegisterValues> {};
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(state,
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
    static_assert(std::same_as<decltype(row.template get<ObjectIndex>(size_t { 0 })), ObjectIndex>);
    EXPECT_EQ(object_values(row), raw);
    EXPECT_TRUE(result.contains(std::tuple { constant.get_index(), local.get_index() }));

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
    constexpr auto word_bits = ygg::uint_t(std::numeric_limits<ygg::uint_t>::digits);
    constexpr auto num_objects = 2 * word_bits + 3;
    constexpr auto last = num_objects - 1;
    const auto name = [](ygg::uint_t i) { return "v" + std::to_string(1000 + i); };
    auto domain_source = std::string("(define (domain bit-boundaries) (:requirements :strips) (:constants");
    for (ygg::uint_t i = 0; i < num_objects; ++i)
        domain_source += " " + name(i);
    domain_source += ") (:predicates (marked ?x) (edge ?x ?y)))";
    auto task_source = std::string("(define (problem bit-boundaries-task) (:domain bit-boundaries) (:init");
    for (const auto i : { ygg::uint_t(0), word_bits - 1, word_bits, last })
        task_source += " (marked " + name(i) + ")";
    for (const auto& [from, to] : std::array { std::pair { ygg::uint_t(0), word_bits - 1 },
                                               std::pair { ygg::uint_t(0), word_bits },
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
    for (const auto i : { ygg::uint_t(0), word_bits - 1, word_bits, last })
    {
        ASSERT_EQ(domain.get_constants()[i].get_name(), name(i));
        ASSERT_EQ(ygg::uint_t(domain.get_constants()[i].get_index()), i);
    }
    auto repository = kr::dl::ConstructorRepositoryFactoryFor<Ext>().create(task->get_repository());
    kr::dl::semantics::Builder builder;
    auto denotations = kr::dl::semantics::DenotationRepositoryFactory().create(task->get_repository());
    kr::dl::semantics::EvaluationStorage<Ext> storage(denotations);
    ygg::Data<kr::dl::semantics::CallArguments> arguments;
    ygg::Data<kr::dl::semantics::RegisterValues> registers;
    auto context = kr::dl::semantics::StateEvaluationContext<Ext, Kind>(initial.get_state(),
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
    for (const auto i : { ygg::uint_t(0), word_bits - 1, word_bits, last })
        EXPECT_TRUE(selected.contains(std::tuple { ObjectIndex(i) }));
    EXPECT_EQ(query("(q_concept x (c_top))").size(), num_objects);
    EXPECT_TRUE(query("(q_concept x (c_bot))").empty());
    const auto identity = query("(q_role (x y) (r_identity (c_top)))");
    EXPECT_EQ(identity.size(), num_objects);
    EXPECT_TRUE(identity.contains(std::tuple { ObjectIndex(last), ObjectIndex(last) }));
    EXPECT_TRUE(query("(q_role (x y) (r_identity (c_bot)))").empty());

    const auto inverse = query("(q_role (x y) (r_inverse " + edge + "))");
    ASSERT_EQ(inverse.size(), 4);
    EXPECT_TRUE(inverse.contains(std::tuple { ObjectIndex(word_bits - 1), ObjectIndex(0) }));
    EXPECT_TRUE(inverse.contains(std::tuple { ObjectIndex(word_bits), ObjectIndex(0) }));
    EXPECT_TRUE(inverse.contains(std::tuple { ObjectIndex(last), ObjectIndex(word_bits - 1) }));
    EXPECT_TRUE(inverse.contains(std::tuple { ObjectIndex(last), ObjectIndex(word_bits) }));
    const auto composition = query("(q_role (x y) (r_composition " + edge + " " + edge + "))");
    ASSERT_EQ(composition.size(), 1);
    EXPECT_TRUE(composition.contains(std::tuple { ObjectIndex(0), ObjectIndex(last) }));
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

}  // namespace runir::tests
