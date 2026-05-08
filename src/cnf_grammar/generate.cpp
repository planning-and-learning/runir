#include "runir/cnf_grammar/generate.hpp"

#include "runir/canonicalization.hpp"
#include "runir/semantics/builder.hpp"
#include "runir/semantics/denotation_repository.hpp"
#include "runir/semantics/evaluation.hpp"
#include "runir/semantics/evaluation_workspace.hpp"

#include <chrono>
#include <concepts>
#include <cstdint>
#include <map>
#include <optional>
#include <type_traits>
#include <tyr/common/chrono.hpp>
#include <tyr/planning/ground_task.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>
#include <utility>
#include <vector>

namespace runir::cnf_grammar
{
namespace
{

template<typename T>
auto intern(runir::ConstructorRepository& repository, tyr::Data<T>& data)
{
    runir::canonicalize(data);
    return repository.get_or_create(data).first;
}

template<runir::CategoryTag Category, typename T>
auto intern_constructor(runir::ConstructorRepository& repository, tyr::Index<T> index)
{
    tyr::Data<runir::Constructor<Category>> data(index);
    return intern(repository, data);
}

class GeneratedSentences
{
private:
    template<runir::CategoryTag Category>
    using ConstructorIndices = std::vector<tyr::Index<runir::Constructor<Category>>>;

    template<runir::CategoryTag Category>
    using ConstructorsByComplexity = std::vector<ConstructorIndices<Category>>;

    template<runir::CategoryTag Category>
    using ConstructorsByNonTerminal = std::map<std::uint64_t, ConstructorsByComplexity<Category>>;

    ConstructorsByNonTerminal<runir::ConceptTag> m_concepts;
    ConstructorsByNonTerminal<runir::RoleTag> m_roles;
    ConstructorsByNonTerminal<runir::BooleanTag> m_booleans;
    ConstructorsByNonTerminal<runir::NumericalTag> m_numericals;

    template<runir::CategoryTag Category>
    auto& get_map() noexcept
    {
        if constexpr (std::same_as<Category, runir::ConceptTag>)
            return m_concepts;
        else if constexpr (std::same_as<Category, runir::RoleTag>)
            return m_roles;
        else if constexpr (std::same_as<Category, runir::BooleanTag>)
            return m_booleans;
        else if constexpr (std::same_as<Category, runir::NumericalTag>)
            return m_numericals;
    }

    template<runir::CategoryTag Category>
    const auto& get_map() const noexcept
    {
        if constexpr (std::same_as<Category, runir::ConceptTag>)
            return m_concepts;
        else if constexpr (std::same_as<Category, runir::RoleTag>)
            return m_roles;
        else if constexpr (std::same_as<Category, runir::BooleanTag>)
            return m_booleans;
        else if constexpr (std::same_as<Category, runir::NumericalTag>)
            return m_numericals;
    }

    template<runir::CategoryTag Category>
    static const ConstructorIndices<Category>& empty_constructors()
    {
        static const auto empty = ConstructorIndices<Category> {};
        return empty;
    }

public:
    template<runir::CategoryTag Category>
    ConstructorIndices<Category>& get(NonTerminalView<Category> nonterminal, size_t complexity)
    {
        auto& by_complexity = get_map<Category>()[static_cast<std::uint64_t>(tyr::uint_t(nonterminal.get_index()))];
        if (complexity >= by_complexity.size())
            by_complexity.resize(complexity + 1);
        return by_complexity[complexity];
    }

    template<runir::CategoryTag Category>
    const ConstructorIndices<Category>& get(NonTerminalView<Category> nonterminal, size_t complexity) const
    {
        const auto& map = get_map<Category>();
        const auto it = map.find(static_cast<std::uint64_t>(tyr::uint_t(nonterminal.get_index())));
        if (it == map.end() || complexity >= it->second.size())
            return empty_constructors<Category>();
        return it->second[complexity];
    }
};

class Timeout
{
private:
    std::optional<tyr::CountdownWatch> m_stopwatch;

public:
    explicit Timeout(const GenerateOptions& options) : m_stopwatch(options.max_time ? std::optional<tyr::CountdownWatch>(*options.max_time) : std::nullopt) {}

    bool has_finished() const { return m_stopwatch && m_stopwatch->has_finished(); }
};

template<tyr::planning::TaskKind Kind>
class Pruning
{
private:
    const std::vector<tyr::planning::StateView<Kind>>& m_states;
    runir::semantics::Builder m_builder;
    runir::semantics::DenotationRepository m_denotation_repository;
    runir::semantics::EvaluationWorkspace m_workspace;

public:
    explicit Pruning(const std::vector<tyr::planning::StateView<Kind>>& states) : m_states(states), m_builder(), m_denotation_repository(0), m_workspace() {}

    template<runir::CategoryTag Category>
    bool should_prune(runir::ConstructorView<Category> constructor)
    {
        if (m_states.empty())
            return false;

        auto created = false;

        for (auto state : m_states)
        {
            auto context = runir::semantics::EvaluationContext<Kind>(state, m_builder, m_denotation_repository);
            auto denotation = runir::semantics::evaluate_impl(constructor, context, m_workspace);
            created |= context.get_denotation_repository().get_or_create(*denotation).second;
        }

        return !created;
    }
};

template<tyr::planning::TaskKind Kind>
class Generator
{
private:
    GrammarView m_grammar;
    const std::vector<tyr::planning::StateView<Kind>>& m_states;
    runir::ConstructorRepository& m_output_repository;
    const GenerateOptions& m_options;
    GenerateResults m_result;
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

    template<typename T>
    auto intern_concrete(tyr::Data<T>& data)
    {
        return intern(m_output_repository, data);
    }

    template<runir::CategoryTag Category, typename T>
    auto intern_wrapped(tyr::Data<T>& data)
    {
        return intern_constructor<Category>(m_output_repository, intern_concrete(data).get_index());
    }

    template<runir::CategoryTag Category>
    void keep(NonTerminalView<Category> lhs, tyr::Index<runir::Constructor<Category>> constructor)
    {
        m_sentences.get(lhs, m_complexity).push_back(constructor);
    }

    template<runir::CategoryTag Category>
    bool maybe_keep(NonTerminalView<Category> lhs, runir::ConstructorView<Category> constructor)
    {
        ++m_result.statistics.num_generated;

        if (m_pruning.should_prune(constructor))
        {
            ++m_result.statistics.num_pruned;
            return false;
        }

        ++m_result.statistics.num_kept;
        keep(lhs, constructor.get_index());
        return true;
    }

    template<runir::CategoryTag Category, typename F>
    bool generate_nullary(NonTerminalView<Category> lhs, F&& make)
    {
        if (m_complexity != 1)
            return true;

        maybe_keep(lhs, std::forward<F>(make)());
        return !out_of_time();
    }

    template<runir::CategoryTag Category, typename ChildCategory, typename F>
    bool generate_unary(NonTerminalView<Category> lhs, NonTerminalView<ChildCategory> arg, F&& make)
    {
        if (m_complexity < 2)
            return true;

        for (auto child : m_sentences.get(arg, m_complexity - 1))
        {
            maybe_keep(lhs, std::forward<F>(make)(child));
            if (out_of_time())
                return false;
        }

        return true;
    }

    template<runir::CategoryTag Category, typename LhsCategory, typename RhsCategory, typename F>
    bool
    generate_binary(NonTerminalView<Category> lhs, NonTerminalView<LhsCategory> child_lhs, NonTerminalView<RhsCategory> child_rhs, bool commutative, F&& make)
    {
        if (m_complexity < 3)
            return true;

        for (size_t i = 1; i < m_complexity - 1; ++i)
        {
            const auto j = m_complexity - i - 1;
            if (commutative && i > j)
                continue;

            for (auto lhs_constructor : m_sentences.get(child_lhs, i))
            {
                for (auto rhs_constructor : m_sentences.get(child_rhs, j))
                {
                    maybe_keep(lhs, std::forward<F>(make)(lhs_constructor, rhs_constructor));
                    if (out_of_time())
                        return false;
                }
            }
        }

        return true;
    }

    template<typename F>
    bool generate_ternary(NonTerminalView<runir::NumericalTag> lhs,
                          NonTerminalView<runir::ConceptTag> child_lhs,
                          NonTerminalView<runir::RoleTag> child_mid,
                          NonTerminalView<runir::ConceptTag> child_rhs,
                          F&& make)
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
                            maybe_keep(lhs, std::forward<F>(make)(lhs_constructor, mid_constructor, rhs_constructor));
                            if (out_of_time())
                                return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    template<runir::CategoryTag Category>
    bool generate_rule(DerivationRuleView<Category> rule)
    {
        return rule.get_rhs().get_variant().apply([&](auto constructor) { return generate_constructor(rule.get_lhs(), constructor); });
    }

    template<runir::CategoryTag Category>
    bool generate_rule(SubstitutionRuleView<Category> rule)
    {
        const auto& source = m_sentences.get(rule.get_rhs(), m_complexity);
        auto& target = m_sentences.get(rule.get_lhs(), m_complexity);
        target.insert(target.end(), source.begin(), source.end());
        return !out_of_time();
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Concept<runir::AtomicStateTag<T>>> data(constructor.get_data().predicate, constructor.get_data().polarity);
                                    return intern_wrapped<runir::ConceptTag>(data);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Concept<runir::AtomicGoalTag<T>>> data(constructor.get_data().predicate, constructor.get_data().polarity);
                                    return intern_wrapped<runir::ConceptTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::BotTag>)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Concept<runir::BotTag>> data;
                                    return intern_wrapped<runir::ConceptTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::TopTag>)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Concept<runir::TopTag>> data;
                                    return intern_wrapped<runir::ConceptTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::NominalTag> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Concept<runir::NominalTag>> data(constructor.get_data().object);
                                    return intern_wrapped<runir::ConceptTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::IntersectionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Concept<runir::IntersectionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::UnionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Concept<runir::UnionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::NegationTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::Concept<runir::NegationTag>> data(arg);
                                  return intern_wrapped<runir::ConceptTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::ValueRestrictionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Concept<runir::ValueRestrictionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::ExistentialQuantificationTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Concept<runir::ExistentialQuantificationTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::RoleValueMapContainmentTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Concept<runir::RoleValueMapContainmentTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::ConceptTag> lhs, ConceptView<runir::RoleValueMapEqualityTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Concept<runir::RoleValueMapEqualityTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::ConceptTag>(data);
                               });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Role<runir::AtomicStateTag<T>>> data(constructor.get_data().predicate, constructor.get_data().polarity);
                                    return intern_wrapped<runir::RoleTag>(data);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Role<runir::AtomicGoalTag<T>>> data(constructor.get_data().predicate, constructor.get_data().polarity);
                                    return intern_wrapped<runir::RoleTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::UniversalTag>)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Role<runir::UniversalTag>> data;
                                    return intern_wrapped<runir::RoleTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::IntersectionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Role<runir::IntersectionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::RoleTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::UnionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Role<runir::UnionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::RoleTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::ComplementTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::Role<runir::ComplementTag>> data(arg);
                                  return intern_wrapped<runir::RoleTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::InverseTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::Role<runir::InverseTag>> data(arg);
                                  return intern_wrapped<runir::RoleTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::CompositionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Role<runir::CompositionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::RoleTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::TransitiveClosureTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::Role<runir::TransitiveClosureTag>> data(arg);
                                  return intern_wrapped<runir::RoleTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::ReflexiveTransitiveClosureTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::Role<runir::ReflexiveTransitiveClosureTag>> data(arg);
                                  return intern_wrapped<runir::RoleTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::RestrictionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::Role<runir::RestrictionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::RoleTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::RoleTag> lhs, RoleView<runir::IdentityTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::Role<runir::IdentityTag>> data(arg);
                                  return intern_wrapped<runir::RoleTag>(data);
                              });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::BooleanTag> lhs, BooleanView<runir::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::Boolean<runir::AtomicStateTag<T>>> data(constructor.get_data().predicate, constructor.get_data().polarity);
                                    return intern_wrapped<runir::BooleanTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::BooleanTag> lhs, BooleanView<runir::NonemptyTag> constructor)
    {
        return constructor.get_arg().apply(
            [&](auto arg)
            {
                using ArgView = std::decay_t<decltype(arg)>;
                if constexpr (std::same_as<ArgView, NonTerminalView<runir::ConceptTag>>)
                {
                    return generate_unary(lhs,
                                          arg,
                                          [&](auto child)
                                          {
                                              tyr::Data<runir::Boolean<runir::NonemptyTag>> data(child);
                                              return intern_wrapped<runir::BooleanTag>(data);
                                          });
                }
                else
                {
                    return generate_unary(lhs,
                                          arg,
                                          [&](auto child)
                                          {
                                              tyr::Data<runir::Boolean<runir::NonemptyTag>> data(child);
                                              return intern_wrapped<runir::BooleanTag>(data);
                                          });
                }
            });
    }

    bool generate_constructor(NonTerminalView<runir::NumericalTag> lhs, NumericalView<runir::CountTag> constructor)
    {
        return constructor.get_arg().apply(
            [&](auto arg)
            {
                using ArgView = std::decay_t<decltype(arg)>;
                if constexpr (std::same_as<ArgView, NonTerminalView<runir::ConceptTag>>)
                {
                    return generate_unary(lhs,
                                          arg,
                                          [&](auto child)
                                          {
                                              tyr::Data<runir::Numerical<runir::CountTag>> data(child);
                                              return intern_wrapped<runir::NumericalTag>(data);
                                          });
                }
                else
                {
                    return generate_unary(lhs,
                                          arg,
                                          [&](auto child)
                                          {
                                              tyr::Data<runir::Numerical<runir::CountTag>> data(child);
                                              return intern_wrapped<runir::NumericalTag>(data);
                                          });
                }
            });
    }

    bool generate_constructor(NonTerminalView<runir::NumericalTag> lhs, NumericalView<runir::DistanceTag> constructor)
    {
        return generate_ternary(lhs,
                                constructor.get_lhs(),
                                constructor.get_mid(),
                                constructor.get_rhs(),
                                [&](auto child_lhs, auto child_mid, auto child_rhs)
                                {
                                    tyr::Data<runir::Numerical<runir::DistanceTag>> data(child_lhs, child_mid, child_rhs);
                                    return intern_wrapped<runir::NumericalTag>(data);
                                });
    }

    template<runir::CategoryTag Category>
    bool generate_category()
    {
        for (auto rule : m_grammar.template get_derivation_rules<Category>())
        {
            if (!generate_rule(rule))
                return false;
        }

        for (auto rule : m_grammar.template get_substitution_rules<Category>())
        {
            if (!generate_rule(rule))
                return false;
        }

        return true;
    }

    template<runir::CategoryTag Category>
    void collect_start()
    {
        if (auto start = m_grammar.template get_start<Category>())
        {
            auto& target = m_result.template get_constructors<Category>();
            for (size_t complexity = 0; complexity <= m_options.max_syntactic_complexity; ++complexity)
                for (auto constructor : m_sentences.get(*start, complexity))
                    target.emplace_back(constructor, m_output_repository);
        }
    }

public:
    Generator(GrammarView grammar,
              const std::vector<tyr::planning::StateView<Kind>>& states,
              runir::ConstructorRepository& output_repository,
              const GenerateOptions& options) :
        m_grammar(grammar),
        m_states(states),
        m_output_repository(output_repository),
        m_options(options),
        m_result(),
        m_sentences(),
        m_pruning(states),
        m_timeout(options)
    {
    }

    GenerateResults run()
    {
        const auto start_time = std::chrono::steady_clock::now();

        for (; m_complexity <= m_options.max_syntactic_complexity; ++m_complexity)
        {
            if (out_of_time())
                break;

            if (!generate_category<runir::ConceptTag>())
                break;
            if (!generate_category<runir::RoleTag>())
                break;
            if (!generate_category<runir::BooleanTag>())
                break;
            if (!generate_category<runir::NumericalTag>())
                break;
        }

        collect_start<runir::ConceptTag>();
        collect_start<runir::RoleTag>();
        collect_start<runir::BooleanTag>();
        collect_start<runir::NumericalTag>();

        m_result.statistics.total_time = std::chrono::steady_clock::now() - start_time;
        return std::move(m_result);
    }
};

}  // namespace

template<tyr::planning::TaskKind Kind>
GenerateResults generate(GrammarView grammar,
                         const std::vector<tyr::planning::StateView<Kind>>& states,
                         runir::ConstructorRepository& output_repository,
                         const GenerateOptions& options)
{
    return Generator<Kind>(grammar, states, output_repository, options).run();
}

template GenerateResults generate<tyr::planning::GroundTag>(GrammarView,
                                                            const std::vector<tyr::planning::StateView<tyr::planning::GroundTag>>&,
                                                            runir::ConstructorRepository&,
                                                            const GenerateOptions&);

template GenerateResults generate<tyr::planning::LiftedTag>(GrammarView,
                                                            const std::vector<tyr::planning::StateView<tyr::planning::LiftedTag>>&,
                                                            runir::ConstructorRepository&,
                                                            const GenerateOptions&);

}
