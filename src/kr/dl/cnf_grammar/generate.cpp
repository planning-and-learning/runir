#include "runir/kr/dl/cnf_grammar/generate.hpp"
#include "runir/kr/dl/cnf_grammar/detail/repository_instantiations.hpp"
#include <urd/repr/sequences/segmented.hpp>

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <chrono>
#include <concepts>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <tyr/planning/state_builder.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/repr/maps/postings.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <urd/repr/pools/pools.hpp>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::dl::cnf_grammar
{
namespace
{

/// The constructors generated so far, by the nonterminal that derives them and their complexity, in the order
/// generated: a multimap per category.
class GeneratedSentences
{
private:
    template<runir::kr::dl::CategoryTag Category>
    using Sentences = urd::Store<urd::map<std::pair<urd::index<NonTerminal<Category>>, size_t>, urd::index<runir::kr::dl::Constructor<Category>>>, urd::multi>;

    std::tuple<Sentences<runir::kr::dl::ConceptTag>,
               Sentences<runir::kr::dl::RoleTag>,
               Sentences<runir::kr::dl::BooleanTag>,
               Sentences<runir::kr::dl::NumericalTag>>
        m_sentences;

public:
    template<runir::kr::dl::CategoryTag Category>
    void add(NonTerminalView<Category> nonterminal, size_t complexity, urd::index<runir::kr::dl::Constructor<Category>> constructor)
    { std::get<Sentences<Category>>(m_sentences).insert({ { nonterminal.index(), complexity }, constructor }); }

    template<runir::kr::dl::CategoryTag Category>
    [[nodiscard]] auto get(NonTerminalView<Category> nonterminal, size_t complexity) const
    { return std::get<Sentences<Category>>(m_sentences).values({ nonterminal.index(), complexity }); }
};

class Timeout
{
private:
    std::optional<ygg::CountdownWatch> m_stopwatch;

public:
    explicit Timeout(const GenerateOptions& options) : m_stopwatch(options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt) {}

    [[nodiscard]] bool has_finished() const { return m_stopwatch && m_stopwatch->has_finished(); }
};

template<runir::kr::dl::CategoryTag Category>
struct SeenDenotationsFor
{
    using Constructor = runir::kr::dl::ConstructorView<Category>;
    using Denotation = runir::kr::dl::semantics::DenotationView<Category>;
    using NonTerminal = NonTerminalView<Category>;

    /// The denotation rows seen (one denotation per state), interned in a urd pool by the denotations' indices: they
    /// all live in the one canonical repository, so an index identifies a denotation. A handle's value identifies a row.
    urd::Store<urd::pool<urd::Store<urd::sequence<urd::index<runir::kr::dl::semantics::Denotation<Category>>>, urd::frozen>>> vectors;
    urd::Store<urd::sequence<urd::index<runir::kr::dl::semantics::Denotation<Category>>>> scratch_indices;
    urd::Store<urd::set<std::pair<NonTerminal, ygg::uint_t>>> signatures;
    urd::Store<urd::set<std::pair<NonTerminal, Constructor>>> constructors;
    urd::Store<urd::sequence<Denotation>> scratch;

    explicit SeenDenotationsFor(size_t num_states) : vectors(), scratch_indices(), signatures(), constructors(), scratch()
    {
        scratch.reserve(num_states);
        scratch_indices.reserve(num_states);
    }
};

struct SeenDenotations
{
    template<runir::kr::dl::CategoryTag Category>
    using Seen = SeenDenotationsFor<Category>;

    std::tuple<Seen<runir::kr::dl::ConceptTag>, Seen<runir::kr::dl::RoleTag>, Seen<runir::kr::dl::BooleanTag>, Seen<runir::kr::dl::NumericalTag>> values;

    explicit SeenDenotations(size_t num_states) :
        values(Seen<runir::kr::dl::ConceptTag>(num_states),
               Seen<runir::kr::dl::RoleTag>(num_states),
               Seen<runir::kr::dl::BooleanTag>(num_states),
               Seen<runir::kr::dl::NumericalTag>(num_states))
    {
    }

    template<runir::kr::dl::CategoryTag Category>
    auto& get() noexcept
    { return std::get<Seen<Category>>(values); }
};

template<tyr::TaskKind Kind>
class Pruning
{
private:
    const urd::Store<urd::sequence<tyr::planning::PackedStateView<Kind>>>* m_states;
    runir::kr::dl::semantics::Builder m_builder;
    runir::kr::dl::semantics::DenotationRepository* m_denotation_repository;
    urd::Store<urd::sequence<runir::kr::dl::semantics::EvaluationStorage>, urd::stable_refs> m_intermediate_storage;
    urd::Store<urd::sequence<runir::kr::dl::semantics::DenotationCaches>> m_denotation_caches;
    SeenDenotations m_seen_denotations;

public:
    Pruning(const urd::Store<urd::sequence<tyr::planning::PackedStateView<Kind>>>& states, runir::kr::dl::semantics::DenotationRepository& denotation_repository) :
        m_states(&states),
        m_builder(),
        m_denotation_repository(&denotation_repository),
        m_intermediate_storage(),
        m_denotation_caches(states.size()),
        m_seen_denotations(states.size())
    {
        for (size_t i = 0; i < states.size(); ++i)
            m_intermediate_storage.emplace_back(denotation_repository);
    }

    template<runir::kr::dl::CategoryTag Category>
    bool should_prune(NonTerminalView<Category> lhs, runir::kr::dl::ConstructorView<Category> constructor)
    {
        auto& seen = m_seen_denotations.template get<Category>();
        if (m_states->empty())
            return !seen.constructors.insert(std::pair(lhs, constructor)).second;

        const auto was_cached = m_denotation_caches.front().template get<Category>(constructor.is_static()).contains(constructor);
        seen.scratch.clear();

        for (size_t i = 0; i < m_states->size(); ++i)
        {
            auto context = runir::kr::dl::semantics::StateEvaluationContext<Kind>((*m_states)[i].unpack(),
                                                                                  m_builder,
                                                                                  m_denotation_caches[i],
                                                                                  *m_denotation_repository,
                                                                                  m_intermediate_storage[i]);
            // Roots share one canonical repository across example states; their
            // intermediate results remain in the per-state reusable storage.
            seen.scratch.push_back(runir::kr::dl::semantics::evaluate<Kind>(constructor, context));
        }

        seen.scratch_indices.clear();
        for (const auto& denotation : seen.scratch)
            seen.scratch_indices.push_back(denotation.index());
        const auto vector = seen.vectors.insert(seen.scratch_indices).first.value;
        const auto is_novel = seen.signatures.insert(std::pair(lhs, vector)).second;

        if (!is_novel && !was_cached)
            for (auto& caches : m_denotation_caches)
                caches.template get<Category>(constructor.is_static()).erase(constructor);

        return !is_novel;
    }
};

template<tyr::TaskKind Kind>
class Generator
{
private:
    GrammarView m_grammar;
    runir::kr::dl::ConstructorRepository* m_output_repository;
    const GenerateOptions* m_options;
    runir::kr::dl::Builder m_builder;
    GenerateResultsFor m_result;
    GeneratedSentences m_sentences;
    Pruning<Kind> m_pruning;
    Timeout m_timeout;
    size_t m_complexity = 1;

    bool out_of_time()
    {
        if (!m_timeout.has_finished())
            return false;

        m_result.status = GenerateStatus::OUT_OF_TIME;
        return true;
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Initialize>
    auto intern_wrapped(Initialize&& initialize)
    {
        auto data = runir::kr::dl::checkout<T>(m_builder);
        std::forward<Initialize>(initialize)(*data);
        const auto concrete = runir::kr::dl::insert(*m_output_repository, *data).first;

        auto wrapper = runir::kr::dl::checkout<runir::kr::dl::Constructor<Category>>(m_builder);
        wrapper->variant = concrete.index();
        return runir::kr::dl::insert(*m_output_repository, *wrapper).first;
    }

    template<runir::kr::dl::CategoryTag Category, typename T>
    auto intern_nullary()
    {
        return intern_wrapped<Category, T>([](auto&) {});
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Arg>
    auto intern_unary(Arg arg)
    {
        return intern_wrapped<Category, T>([&](auto& data) { data.arg = arg; });
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Lhs, typename Rhs>
    auto intern_binary(Lhs lhs, Rhs rhs)
    {
        return intern_wrapped<Category, T>(
            [&](auto& data)
            {
                data.lhs = lhs;
                data.rhs = rhs;
            });
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Predicate>
    auto intern_predicate(Predicate predicate, bool polarity)
    {
        return intern_wrapped<Category, T>(
            [&](auto& data)
            {
                data.predicate = predicate;
                data.polarity = polarity;
            });
    }

    template<runir::kr::dl::CategoryTag Category>
    void keep(NonTerminalView<Category> lhs, urd::index<runir::kr::dl::Constructor<Category>> constructor)
    { m_sentences.add(lhs, m_complexity, constructor); }

    template<runir::kr::dl::CategoryTag Category>
    bool maybe_keep(NonTerminalView<Category> lhs, runir::kr::dl::ConstructorView<Category> constructor)
    {
        ++m_result.statistics.num_generated;

        if (m_pruning.should_prune(lhs, constructor))
        {
            ++m_result.statistics.num_pruned;
            return false;
        }

        ++m_result.statistics.num_kept;
        keep(lhs, constructor.index());
        return true;
    }

    template<runir::kr::dl::CategoryTag Category, typename F>
    bool generate_nullary(NonTerminalView<Category> lhs, F&& make)
    {
        if (m_complexity != 1)
            return true;

        maybe_keep(lhs, std::forward<F>(make)());
        return !out_of_time();
    }

    template<runir::kr::dl::CategoryTag Category, typename ChildCategory, typename F>
    bool generate_unary(NonTerminalView<Category> lhs, NonTerminalView<ChildCategory> arg, F make)
    {
        if (m_complexity < 2)
            return true;

        for (auto child : m_sentences.get(arg, m_complexity - 1))
        {
            maybe_keep(lhs, make(child));
            if (out_of_time())
                return false;
        }

        return true;
    }

    template<runir::kr::dl::CategoryTag Category, typename LhsCategory, typename RhsCategory, typename F>
    bool
    generate_binary(NonTerminalView<Category> lhs, NonTerminalView<LhsCategory> child_lhs, NonTerminalView<RhsCategory> child_rhs, bool commutative, F make)
    {
        if (m_complexity < 3)
            return true;

        bool operands_are_interchangeable = false;
        if constexpr (std::same_as<LhsCategory, RhsCategory>)
            operands_are_interchangeable = child_lhs == child_rhs;

        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            const auto j = m_complexity - i - 1;
            if (commutative && operands_are_interchangeable && i > j)
                continue;

            for (auto lhs_constructor : m_sentences.get(child_lhs, i))
            {
                for (auto rhs_constructor : m_sentences.get(child_rhs, j))
                {
                    if constexpr (std::same_as<LhsCategory, RhsCategory>)
                        if (commutative && operands_are_interchangeable && i == j && rhs_constructor < lhs_constructor)
                            continue;

                    maybe_keep(lhs, make(lhs_constructor, rhs_constructor));
                    if (out_of_time())
                        return false;
                }
            }
        }

        return true;
    }

    template<typename F>
    bool generate_ternary(NonTerminalView<runir::kr::dl::NumericalTag> lhs,
                          NonTerminalView<runir::kr::dl::ConceptTag> child_lhs,
                          NonTerminalView<runir::kr::dl::RoleTag> child_mid,
                          NonTerminalView<runir::kr::dl::ConceptTag> child_rhs,
                          F make)
    {
        if (m_complexity < 4)
            return true;

        for (size_t i = 1; i < m_complexity - 2; ++i)
        {
            for (size_t j = 1; j < m_complexity - i - 1; ++j)
            {
                const auto k = m_complexity - i - j - 1;

                for (auto lhs_constructor : m_sentences.get(child_lhs, i))
                {
                    for (auto mid_constructor : m_sentences.get(child_mid, j))
                    {
                        for (auto rhs_constructor : m_sentences.get(child_rhs, k))
                        {
                            maybe_keep(lhs, make(lhs_constructor, mid_constructor, rhs_constructor));
                            if (out_of_time())
                                return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    template<runir::kr::dl::CategoryTag Category>
    bool generate_rule(DerivationRuleView<Category> rule)
    {
        return std::visit([&](auto constructor) { return generate_constructor(rule.get_lhs(), constructor); }, rule.get_rhs());
    }

    template<runir::kr::dl::CategoryTag Category>
    bool propagate(SubstitutionRuleView<Category> rule, bool& changed)
    {
        if (rule.get_lhs() == rule.get_rhs())
            return !out_of_time();

        const auto source = m_sentences.get(rule.get_rhs(), m_complexity);
        for (auto constructor : source)
        {
            auto view = m_output_repository->view(constructor);
            if (!m_pruning.should_prune(rule.get_lhs(), view))
            {
                keep(rule.get_lhs(), constructor);
                changed = true;
            }

            if (out_of_time())
                return false;
        }

        return !out_of_time();
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicStateTag<T>>>(
                                        constructor.data().predicate,
                                        constructor.data().polarity);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicGoalTag<T>>>(
                                        constructor.data().predicate,
                                        constructor.data().polarity);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::BotTag>)
    {
        return generate_nullary(lhs, [&] { return intern_nullary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::BotTag>>(); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::TopTag>)
    {
        return generate_nullary(lhs, [&] { return intern_nullary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::TopTag>>(); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::NominalTag> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::NominalTag>>(
                                        [&](auto& data) { data.object = constructor.data().object; });
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::IntersectionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            true,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::IntersectionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::UnionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::UnionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::NegationTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg) { return intern_unary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::NegationTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::ValueRestrictionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            false,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::ValueRestrictionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::ExistentialQuantificationTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            false,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::ExistentialQuantificationTag>>(child_lhs, child_rhs); });
    }

    template<typename Tag>
    bool generate_number_restriction(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<Tag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_role(),
                              [&](auto role)
                              {
                                  return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, Tag>>(
                                      [&](auto& data)
                                      {
                                          data.n = constructor.get_n();
                                          data.role = role;
                                      });
                              });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AtLeastNumberRestrictionTag> constructor)
    { return generate_number_restriction(lhs, constructor); }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AtMostNumberRestrictionTag> constructor)
    { return generate_number_restriction(lhs, constructor); }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::ExactNumberRestrictionTag> constructor)
    { return generate_number_restriction(lhs, constructor); }

    template<typename Tag>
    bool generate_qualified_number_restriction(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<Tag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_role(),
                               constructor.get_concept(),
                               false,
                               [&](auto role, auto concept_)
                               {
                                   return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, Tag>>(
                                       [&](auto& data)
                                       {
                                           data.n = constructor.get_n();
                                           data.role = role;
                                           data.concept_ = concept_;
                                       });
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> constructor)
    { return generate_qualified_number_restriction(lhs, constructor); }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::QualifiedAtMostNumberRestrictionTag> constructor)
    { return generate_qualified_number_restriction(lhs, constructor); }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::QualifiedExactNumberRestrictionTag> constructor)
    { return generate_qualified_number_restriction(lhs, constructor); }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::RoleValueMapTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            false,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::RoleValueMapTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AgreementTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::AgreementTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::RoleFillersTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_role(),
                              [&](auto role)
                              {
                                  return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::RoleFillersTag>>(
                                      [&](auto& data)
                                      {
                                          data.role = role;
                                          for (auto object : constructor.data().objects)
                                              data.objects.push_back(object);
                                      });
                              });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::OneOfTag> constructor)
    {
        return generate_nullary(lhs,
                                [&]()
                                {
                                    return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::OneOfTag>>(
                                        [&](auto& data)
                                        {
                                            for (auto object : constructor.data().objects)
                                                data.objects.push_back(object);
                                        });
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicStateTag<T>>>(
                                        constructor.data().predicate,
                                        constructor.data().polarity);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicGoalTag<T>>>(
                                        constructor.data().predicate,
                                        constructor.data().polarity);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::UniversalTag>)
    {
        return generate_nullary(lhs, [&] { return intern_nullary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::UniversalTag>>(); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::IntersectionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               { return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::IntersectionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::UnionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               { return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::UnionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::ComplementTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg) { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::ComplementTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::InverseTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg) { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::InverseTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::CompositionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               { return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::CompositionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::TransitiveClosureTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg) { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::TransitiveClosureTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::ReflexiveTransitiveClosureTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::ReflexiveTransitiveClosureTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::RestrictionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               { return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::RestrictionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::IdentityTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg) { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::IdentityTag>>(arg); });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::BooleanTag> lhs, BooleanView<runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicStateTag<T>>>(
                                        constructor.data().predicate,
                                        constructor.data().polarity);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::BooleanTag> lhs, BooleanView<runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicGoalTag<T>>>(
                                        constructor.data().predicate,
                                        constructor.data().polarity);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::BooleanTag> lhs, BooleanView<runir::kr::dl::NonemptyTag> constructor)
    {
        return std::visit(
            [&](auto arg)
            {
                return generate_unary(lhs,
                                      arg,
                                      [&](auto child)
                                      { return intern_unary<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, runir::kr::dl::NonemptyTag>>(child); });
            },
            constructor.get_arg());
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::NumericalTag> lhs, NumericalView<runir::kr::dl::CountTag> constructor)
    {
        return std::visit(
            [&](auto arg)
            {
                return generate_unary(lhs,
                                      arg,
                                      [&](auto child)
                                      { return intern_unary<runir::kr::dl::NumericalTag, runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::CountTag>>(child); });
            },
            constructor.get_arg());
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::NumericalTag> lhs, NumericalView<runir::kr::dl::DistanceTag> constructor)
    {
        return generate_ternary(lhs,
                                constructor.get_lhs(),
                                constructor.get_mid(),
                                constructor.get_rhs(),
                                [&](auto child_lhs, auto child_mid, auto child_rhs)
                                {
                                    return intern_wrapped<runir::kr::dl::NumericalTag, runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>>(
                                        [&](auto& data)
                                        {
                                            data.lhs = child_lhs;
                                            data.mid = child_mid;
                                            data.rhs = child_rhs;
                                        });
                                });
    }

    template<runir::kr::dl::CategoryTag Category>
    bool generate_category()
    {
        for (auto rule : m_grammar.template get_derivation_rules<Category>())
        {
            if (!generate_rule(rule))
                return false;
        }

        // ponytail: rescan unit rules; use a worklist only if substitution-heavy grammars profile poorly.
        auto changed = true;
        while (changed)
        {
            changed = false;
            for (auto rule : m_grammar.template get_substitution_rules<Category>())
                if (!propagate(rule, changed))
                    return false;
        }

        return true;
    }

    template<runir::kr::dl::CategoryTag Category>
    void collect_start()
    {
        if (auto start = m_grammar.template get_start<Category>())
        {
            auto& target = m_result.template get_constructors<Category>();
            for (size_t complexity = 0; complexity <= m_options->max_syntactic_complexity; ++complexity)
                for (auto constructor : m_sentences.get(*start, complexity))
                    target.push_back(m_output_repository->view(constructor));
        }
    }

public:
    Generator(GrammarView grammar,
              const urd::Store<urd::sequence<tyr::planning::PackedStateView<Kind>>>& states,
              runir::kr::dl::ConstructorRepository& output_repository,
              runir::kr::dl::semantics::DenotationRepository& denotation_repository,
              const GenerateOptions& options) :
        m_grammar(grammar),
        m_output_repository(&output_repository),
        m_options(&options),
        m_builder(),
        m_result(),
        m_sentences(),
        m_pruning(states, denotation_repository),
        m_timeout(options)
    {
    }

    GenerateResultsFor run()
    {
        const auto start_time = std::chrono::steady_clock::now();

        for (; m_complexity <= m_options->max_syntactic_complexity; ++m_complexity)
        {
            if (out_of_time())
                break;

            if (!generate_category<runir::kr::dl::ConceptTag>())
                break;
            if (!generate_category<runir::kr::dl::RoleTag>())
                break;
            if (!generate_category<runir::kr::dl::BooleanTag>())
                break;
            if (!generate_category<runir::kr::dl::NumericalTag>())
                break;
        }

        collect_start<runir::kr::dl::ConceptTag>();
        collect_start<runir::kr::dl::RoleTag>();
        collect_start<runir::kr::dl::BooleanTag>();
        collect_start<runir::kr::dl::NumericalTag>();

        m_result.statistics.total_time = std::chrono::steady_clock::now() - start_time;
        return std::move(m_result);
    }
};

}  // namespace
template<tyr::TaskKind Kind>
GenerateResultsFor generate(GrammarView grammar,
                            const urd::Store<urd::sequence<tyr::planning::PackedStateView<Kind>>>& states,
                            runir::kr::dl::ConstructorRepository& output_repository,
                            runir::kr::dl::semantics::DenotationRepository& denotation_repository,
                            const GenerateOptions& options)
{
    const auto& planning_repository = output_repository.get_planning_repository_ptr();
    if (&grammar.context().get_planning_repository().get_root() != &planning_repository->get_root())
        throw std::invalid_argument("generate requires grammar and output repository for the same planning domain");

    for (const auto& state : states)
    {
        const auto& task = state.get_state_repository().get_task();
        if (task->get_repository() != planning_repository && task->get_domain().get_repository() != planning_repository)
            throw std::invalid_argument("generate requires states and output repository for the same planning domain");
        if (task->get_repository().get() != &denotation_repository.get_formalism_repository())
            throw std::invalid_argument("generate requires states and denotation repository for the same planning task");
    }

    return Generator<Kind>(grammar, states, output_repository, denotation_repository, options).run();
}

template GenerateResultsFor generate<tyr::GroundTag>(GrammarView,
                                                     const urd::Store<urd::sequence<tyr::planning::PackedStateView<tyr::GroundTag>>>&,
                                                     runir::kr::dl::ConstructorRepository&,
                                                     runir::kr::dl::semantics::DenotationRepository&,
                                                     const GenerateOptions&);

template GenerateResultsFor generate<tyr::LiftedTag>(GrammarView,
                                                     const urd::Store<urd::sequence<tyr::planning::PackedStateView<tyr::LiftedTag>>>&,
                                                     runir::kr::dl::ConstructorRepository&,
                                                     runir::kr::dl::semantics::DenotationRepository&,
                                                     const GenerateOptions&);
}
