#include "runir/kr/dl/cnf_grammar/generate.hpp"

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"

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

namespace runir::kr::dl::cnf_grammar
{
namespace
{

template<typename T>
auto intern(runir::kr::dl::ConstructorRepository& repository, tyr::Data<T>& data)
{
    runir::kr::dl::canonicalize(data);
    return repository.get_or_create(data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(runir::kr::dl::ConstructorRepository& repository, tyr::Index<T> index)
{
    tyr::Data<runir::kr::dl::Constructor<Category>> data(index);
    return intern(repository, data);
}

class GeneratedSentences
{
private:
    template<runir::kr::dl::CategoryTag Category>
    using ConstructorIndices = std::vector<tyr::Index<runir::kr::dl::Constructor<Category>>>;

    template<runir::kr::dl::CategoryTag Category>
    using ConstructorsByComplexity = std::vector<ConstructorIndices<Category>>;

    template<runir::kr::dl::CategoryTag Category>
    using ConstructorsByNonTerminal = std::map<std::uint64_t, ConstructorsByComplexity<Category>>;

    ConstructorsByNonTerminal<runir::kr::dl::ConceptTag> m_concepts;
    ConstructorsByNonTerminal<runir::kr::dl::RoleTag> m_roles;
    ConstructorsByNonTerminal<runir::kr::dl::BooleanTag> m_booleans;
    ConstructorsByNonTerminal<runir::kr::dl::NumericalTag> m_numericals;

    template<runir::kr::dl::CategoryTag Category>
    auto& get_map() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return m_concepts;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return m_roles;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return m_booleans;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return m_numericals;
    }

    template<runir::kr::dl::CategoryTag Category>
    const auto& get_map() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return m_concepts;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return m_roles;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return m_booleans;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return m_numericals;
    }

    template<runir::kr::dl::CategoryTag Category>
    static const ConstructorIndices<Category>& empty_constructors()
    {
        static const auto empty = ConstructorIndices<Category> {};
        return empty;
    }

public:
    template<runir::kr::dl::CategoryTag Category>
    ConstructorIndices<Category>& get(NonTerminalView<Category> nonterminal, size_t complexity)
    {
        auto& by_complexity = get_map<Category>()[static_cast<std::uint64_t>(tyr::uint_t(nonterminal.get_index()))];
        if (complexity >= by_complexity.size())
            by_complexity.resize(complexity + 1);
        return by_complexity[complexity];
    }

    template<runir::kr::dl::CategoryTag Category>
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
    runir::kr::dl::semantics::Builder m_builder;
    runir::kr::dl::semantics::DenotationRepository m_denotation_repository;
    runir::kr::dl::semantics::EvaluationWorkspace m_workspace;

public:
    explicit Pruning(const std::vector<tyr::planning::StateView<Kind>>& states) : m_states(states), m_builder(), m_denotation_repository(0), m_workspace() {}

    template<runir::kr::dl::CategoryTag Category>
    bool should_prune(runir::kr::dl::ConstructorView<Category> constructor)
    {
        if (m_states.empty())
            return false;

        auto created = false;

        for (auto state : m_states)
        {
            auto context = runir::kr::dl::semantics::EvaluationContext<Kind>(state, m_builder, m_denotation_repository);
            auto denotation = runir::kr::dl::semantics::evaluate_impl(constructor, context, m_workspace);
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
    runir::kr::dl::ConstructorRepository& m_output_repository;
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

    template<runir::kr::dl::CategoryTag Category, typename T>
    auto intern_wrapped(tyr::Data<T>& data)
    {
        return intern_constructor<Category>(m_output_repository, intern_concrete(data).get_index());
    }

    template<runir::kr::dl::CategoryTag Category>
    void keep(NonTerminalView<Category> lhs, tyr::Index<runir::kr::dl::Constructor<Category>> constructor)
    {
        m_sentences.get(lhs, m_complexity).push_back(constructor);
    }

    template<runir::kr::dl::CategoryTag Category>
    bool maybe_keep(NonTerminalView<Category> lhs, runir::kr::dl::ConstructorView<Category> constructor)
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

    template<runir::kr::dl::CategoryTag Category, typename F>
    bool generate_nullary(NonTerminalView<Category> lhs, F&& make)
    {
        if (m_complexity != 1)
            return true;

        maybe_keep(lhs, std::forward<F>(make)());
        return !out_of_time();
    }

    template<runir::kr::dl::CategoryTag Category, typename ChildCategory, typename F>
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

    template<runir::kr::dl::CategoryTag Category, typename LhsCategory, typename RhsCategory, typename F>
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
    bool generate_ternary(NonTerminalView<runir::kr::dl::NumericalTag> lhs,
                          NonTerminalView<runir::kr::dl::ConceptTag> child_lhs,
                          NonTerminalView<runir::kr::dl::RoleTag> child_mid,
                          NonTerminalView<runir::kr::dl::ConceptTag> child_rhs,
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

    template<runir::kr::dl::CategoryTag Category>
    bool generate_rule(DerivationRuleView<Category> rule)
    {
        return rule.get_rhs().get_variant().apply([&](auto constructor) { return generate_constructor(rule.get_lhs(), constructor); });
    }

    template<runir::kr::dl::CategoryTag Category>
    bool generate_rule(SubstitutionRuleView<Category> rule)
    {
        const auto& source = m_sentences.get(rule.get_rhs(), m_complexity);
        auto& target = m_sentences.get(rule.get_lhs(), m_complexity);
        target.insert(target.end(), source.begin(), source.end());
        return !out_of_time();
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::AtomicStateTag<T>>> data(constructor.get_data().predicate,
                                                                                                             constructor.get_data().polarity);
                                    return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::AtomicGoalTag<T>>> data(constructor.get_data().predicate,
                                                                                                            constructor.get_data().polarity);
                                    return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::BotTag>)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::BotTag>> data;
                                    return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::TopTag>)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::TopTag>> data;
                                    return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::NominalTag> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Concept<runir::kr::dl::NominalTag>> data(constructor.get_data().object);
                                    return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::IntersectionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Concept<runir::kr::dl::IntersectionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::UnionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Concept<runir::kr::dl::UnionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::NegationTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::kr::dl::Concept<runir::kr::dl::NegationTag>> data(arg);
                                  return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::ValueRestrictionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Concept<runir::kr::dl::ValueRestrictionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::ExistentialQuantificationTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Concept<runir::kr::dl::ExistentialQuantificationTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::RoleValueMapContainmentTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Concept<runir::kr::dl::RoleValueMapContainmentTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::RoleValueMapEqualityTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Concept<runir::kr::dl::RoleValueMapEqualityTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::ConceptTag>(data);
                               });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Role<runir::kr::dl::AtomicStateTag<T>>> data(constructor.get_data().predicate,
                                                                                                          constructor.get_data().polarity);
                                    return intern_wrapped<runir::kr::dl::RoleTag>(data);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Role<runir::kr::dl::AtomicGoalTag<T>>> data(constructor.get_data().predicate,
                                                                                                         constructor.get_data().polarity);
                                    return intern_wrapped<runir::kr::dl::RoleTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::UniversalTag>)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Role<runir::kr::dl::UniversalTag>> data;
                                    return intern_wrapped<runir::kr::dl::RoleTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::IntersectionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Role<runir::kr::dl::IntersectionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::RoleTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::UnionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Role<runir::kr::dl::UnionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::RoleTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::ComplementTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::kr::dl::Role<runir::kr::dl::ComplementTag>> data(arg);
                                  return intern_wrapped<runir::kr::dl::RoleTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::InverseTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::kr::dl::Role<runir::kr::dl::InverseTag>> data(arg);
                                  return intern_wrapped<runir::kr::dl::RoleTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::CompositionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Role<runir::kr::dl::CompositionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::RoleTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::TransitiveClosureTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::kr::dl::Role<runir::kr::dl::TransitiveClosureTag>> data(arg);
                                  return intern_wrapped<runir::kr::dl::RoleTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::ReflexiveTransitiveClosureTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::kr::dl::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>> data(arg);
                                  return intern_wrapped<runir::kr::dl::RoleTag>(data);
                              });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::RestrictionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs)
                               {
                                   tyr::Data<runir::kr::dl::Role<runir::kr::dl::RestrictionTag>> data(child_lhs, child_rhs);
                                   return intern_wrapped<runir::kr::dl::RoleTag>(data);
                               });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::IdentityTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              {
                                  tyr::Data<runir::kr::dl::Role<runir::kr::dl::IdentityTag>> data(arg);
                                  return intern_wrapped<runir::kr::dl::RoleTag>(data);
                              });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::BooleanTag> lhs, BooleanView<runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    tyr::Data<runir::kr::dl::Boolean<runir::kr::dl::AtomicStateTag<T>>> data(constructor.get_data().predicate,
                                                                                                             constructor.get_data().polarity);
                                    return intern_wrapped<runir::kr::dl::BooleanTag>(data);
                                });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::BooleanTag> lhs, BooleanView<runir::kr::dl::NonemptyTag> constructor)
    {
        return constructor.get_arg().apply(
            [&](auto arg)
            {
                using ArgView = std::decay_t<decltype(arg)>;
                if constexpr (std::same_as<ArgView, NonTerminalView<runir::kr::dl::ConceptTag>>)
                {
                    return generate_unary(lhs,
                                          arg,
                                          [&](auto child)
                                          {
                                              tyr::Data<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>> data(child);
                                              return intern_wrapped<runir::kr::dl::BooleanTag>(data);
                                          });
                }
                else
                {
                    return generate_unary(lhs,
                                          arg,
                                          [&](auto child)
                                          {
                                              tyr::Data<runir::kr::dl::Boolean<runir::kr::dl::NonemptyTag>> data(child);
                                              return intern_wrapped<runir::kr::dl::BooleanTag>(data);
                                          });
                }
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::NumericalTag> lhs, NumericalView<runir::kr::dl::CountTag> constructor)
    {
        return constructor.get_arg().apply(
            [&](auto arg)
            {
                using ArgView = std::decay_t<decltype(arg)>;
                if constexpr (std::same_as<ArgView, NonTerminalView<runir::kr::dl::ConceptTag>>)
                {
                    return generate_unary(lhs,
                                          arg,
                                          [&](auto child)
                                          {
                                              tyr::Data<runir::kr::dl::Numerical<runir::kr::dl::CountTag>> data(child);
                                              return intern_wrapped<runir::kr::dl::NumericalTag>(data);
                                          });
                }
                else
                {
                    return generate_unary(lhs,
                                          arg,
                                          [&](auto child)
                                          {
                                              tyr::Data<runir::kr::dl::Numerical<runir::kr::dl::CountTag>> data(child);
                                              return intern_wrapped<runir::kr::dl::NumericalTag>(data);
                                          });
                }
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::NumericalTag> lhs, NumericalView<runir::kr::dl::DistanceTag> constructor)
    {
        return generate_ternary(lhs,
                                constructor.get_lhs(),
                                constructor.get_mid(),
                                constructor.get_rhs(),
                                [&](auto child_lhs, auto child_mid, auto child_rhs)
                                {
                                    tyr::Data<runir::kr::dl::Numerical<runir::kr::dl::DistanceTag>> data(child_lhs, child_mid, child_rhs);
                                    return intern_wrapped<runir::kr::dl::NumericalTag>(data);
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

        for (auto rule : m_grammar.template get_substitution_rules<Category>())
        {
            if (!generate_rule(rule))
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
            for (size_t complexity = 0; complexity <= m_options.max_syntactic_complexity; ++complexity)
                for (auto constructor : m_sentences.get(*start, complexity))
                    target.emplace_back(constructor, m_output_repository);
        }
    }

public:
    Generator(GrammarView grammar,
              const std::vector<tyr::planning::StateView<Kind>>& states,
              runir::kr::dl::ConstructorRepository& output_repository,
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

template<tyr::planning::TaskKind Kind>
GenerateResults generate(GrammarView grammar,
                         const std::vector<tyr::planning::StateView<Kind>>& states,
                         runir::kr::dl::ConstructorRepository& output_repository,
                         const GenerateOptions& options)
{
    return Generator<Kind>(grammar, states, output_repository, options).run();
}

template GenerateResults generate<tyr::planning::GroundTag>(GrammarView,
                                                            const std::vector<tyr::planning::StateView<tyr::planning::GroundTag>>&,
                                                            runir::kr::dl::ConstructorRepository&,
                                                            const GenerateOptions&);

template GenerateResults generate<tyr::planning::LiftedTag>(GrammarView,
                                                            const std::vector<tyr::planning::StateView<tyr::planning::LiftedTag>>&,
                                                            runir::kr::dl::ConstructorRepository&,
                                                            const GenerateOptions&);

}
