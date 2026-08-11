#include "runir/kr/dl/cnf_grammar/generate.hpp"

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/base/evaluation_context.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"

#include <chrono>
#include <concepts>
#include <cstdint>
#include <map>
#include <optional>
#include <type_traits>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::dl::cnf_grammar
{
namespace
{

template<runir::kr::dl::FamilyTag Family>
class GeneratedSentences
{
private:
    template<runir::kr::dl::CategoryTag Category>
    using ConstructorIndices = std::vector<ygg::Index<runir::kr::dl::Constructor<Family, Category>>>;

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
    ConstructorIndices<Category>& get(FamilyNonTerminalView<Family, Category> nonterminal, size_t complexity)
    {
        auto& by_complexity = get_map<Category>()[static_cast<std::uint64_t>(ygg::uint_t(nonterminal.get_index()))];
        if (complexity >= by_complexity.size())
            by_complexity.resize(complexity + 1);
        return by_complexity[complexity];
    }

    template<runir::kr::dl::CategoryTag Category>
    const ConstructorIndices<Category>& get(FamilyNonTerminalView<Family, Category> nonterminal, size_t complexity) const
    {
        const auto& map = get_map<Category>();
        const auto it = map.find(static_cast<std::uint64_t>(ygg::uint_t(nonterminal.get_index())));
        if (it == map.end() || complexity >= it->second.size())
            return empty_constructors<Category>();
        return it->second[complexity];
    }
};

class Timeout
{
private:
    std::optional<ygg::CountdownWatch> m_stopwatch;

public:
    explicit Timeout(const GenerateOptions& options) : m_stopwatch(options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt) {}

    bool has_finished() const { return m_stopwatch && m_stopwatch->has_finished(); }
};

template<runir::kr::dl::FamilyTag Family, tyr::TaskKind Kind>
class Pruning
{
private:
    const std::vector<tyr::planning::StateView<Kind>>& m_states;
    runir::kr::dl::semantics::Builder m_builder;
    runir::kr::dl::semantics::DenotationRepositoryFactory m_denotation_repository_factory;
    runir::kr::dl::semantics::DenotationRepository m_denotation_repository;
    runir::kr::dl::semantics::EvaluationWorkspace m_workspace;

public:
    Pruning(const std::vector<tyr::planning::StateView<Kind>>& states, const runir::kr::dl::ConstructorRepositoryFor<Family>& output_repository) :
        m_states(states),
        m_builder(),
        m_denotation_repository_factory(),
        m_denotation_repository(m_denotation_repository_factory.create(output_repository.get_planning_repository_ptr())),
        m_workspace()
    {
    }

    template<runir::kr::dl::CategoryTag Category>
    bool should_prune(runir::kr::dl::FamilyConstructorView<Family, Category> constructor)
    {
        if (m_states.empty())
            return false;

        auto created = false;

        for (auto state : m_states)
        {
            auto context = runir::kr::dl::semantics::EvaluationContext<Family, Kind>(state, m_builder, m_denotation_repository);
            auto denotation = runir::kr::dl::semantics::evaluate_impl(constructor, context, m_workspace);
            auto data = runir::kr::dl::semantics::checkout<runir::kr::dl::semantics::Denotation<Category>>(m_builder);
            runir::kr::dl::semantics::make_data(*denotation, *data);
            if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag> || std::same_as<Category, runir::kr::dl::RoleTag>)
                data->vec_index = m_denotation_repository.get_vector_repository().insert(denotation->blocks);

            const auto [view, was_created] = runir::kr::dl::semantics::get_or_create(context.get_denotation_repository(), *data);
            denotation->index = view.get_index();
            created |= was_created;
        }

        return !created;
    }
};

template<runir::kr::dl::FamilyTag Family, tyr::TaskKind Kind>
class Generator
{
private:
    FamilyGrammarView<Family> m_grammar;
    const std::vector<tyr::planning::StateView<Kind>>& m_states;
    runir::kr::dl::ConstructorRepositoryFor<Family>& m_output_repository;
    const GenerateOptions& m_options;
    runir::kr::dl::Builder<Family> m_builder;
    GenerateResultsFor<Family> m_result;
    GeneratedSentences<Family> m_sentences;
    Pruning<Family, Kind> m_pruning;
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
        const auto concrete = runir::kr::dl::get_or_create(m_output_repository, *data).first;

        auto wrapper = runir::kr::dl::checkout<runir::kr::dl::Constructor<Family, Category>>(m_builder);
        wrapper->value = concrete.get_index();
        return runir::kr::dl::get_or_create(m_output_repository, *wrapper).first;
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
    void keep(FamilyNonTerminalView<Family, Category> lhs, ygg::Index<runir::kr::dl::Constructor<Family, Category>> constructor)
    {
        m_sentences.get(lhs, m_complexity).push_back(constructor);
    }

    template<runir::kr::dl::CategoryTag Category>
    bool maybe_keep(FamilyNonTerminalView<Family, Category> lhs, runir::kr::dl::FamilyConstructorView<Family, Category> constructor)
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
    bool generate_nullary(FamilyNonTerminalView<Family, Category> lhs, F&& make)
    {
        if (m_complexity != 1)
            return true;

        maybe_keep(lhs, std::forward<F>(make)());
        return !out_of_time();
    }

    template<runir::kr::dl::CategoryTag Category, typename ChildCategory, typename F>
    bool generate_unary(FamilyNonTerminalView<Family, Category> lhs, FamilyNonTerminalView<Family, ChildCategory> arg, F&& make)
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
    bool generate_binary(FamilyNonTerminalView<Family, Category> lhs,
                         FamilyNonTerminalView<Family, LhsCategory> child_lhs,
                         FamilyNonTerminalView<Family, RhsCategory> child_rhs,
                         bool commutative,
                         F&& make)
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
    bool generate_ternary(FamilyNonTerminalView<Family, runir::kr::dl::NumericalTag> lhs,
                          FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> child_lhs,
                          FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> child_mid,
                          FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> child_rhs,
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
    bool generate_rule(FamilyDerivationRuleView<Family, Category> rule)
    {
        return ygg::visit([&](auto constructor) { return generate_constructor(rule.get_lhs(), constructor); }, rule.get_rhs().get_variant());
    }

    template<runir::kr::dl::CategoryTag Category>
    bool generate_rule(FamilySubstitutionRuleView<Family, Category> rule)
    {
        const auto& source = m_sentences.get(rule.get_rhs(), m_complexity);
        auto& target = m_sentences.get(rule.get_lhs(), m_complexity);
        target.insert(target.end(), source.begin(), source.end());
        return !out_of_time();
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<T>>>(
                                        constructor.get_data().predicate,
                                        constructor.get_data().polarity);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>>(
                                        constructor.get_data().predicate,
                                        constructor.get_data().polarity);
                                });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, runir::kr::dl::BotTag>)
    {
        return generate_nullary(lhs, [&] { return intern_nullary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::BotTag>>(); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, runir::kr::dl::TopTag>)
    {
        return generate_nullary(lhs, [&] { return intern_nullary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::TopTag>>(); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, runir::kr::dl::NominalTag> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::NominalTag>>(
                                        [&](auto& data) { data.object = constructor.get_data().object; });
                                });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::IntersectionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            true,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::IntersectionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, runir::kr::dl::UnionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            true,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::UnionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, runir::kr::dl::NegationTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              { return intern_unary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::NegationTag>>(arg); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::ValueRestrictionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            false,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::ValueRestrictionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::ExistentialQuantificationTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               false,
                               [&](auto child_lhs, auto child_rhs) {
                                   return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>>(
                                       child_lhs,
                                       child_rhs);
                               });
    }

    template<typename Tag>
    bool generate_number_restriction(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, Tag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_role(),
                              [&](auto role)
                              {
                                  return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, Tag>>(
                                      [&](auto& data)
                                      {
                                          data.n = constructor.get_n();
                                          data.role = role;
                                      });
                              });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::AtLeastNumberRestrictionTag> constructor)
    {
        return generate_number_restriction(lhs, constructor);
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::AtMostNumberRestrictionTag> constructor)
    {
        return generate_number_restriction(lhs, constructor);
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::ExactNumberRestrictionTag> constructor)
    {
        return generate_number_restriction(lhs, constructor);
    }

    template<typename Tag>
    bool generate_qualified_number_restriction(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, Tag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_role(),
                               constructor.get_concept(),
                               false,
                               [&](auto role, auto concept_)
                               {
                                   return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, Tag>>(
                                       [&](auto& data)
                                       {
                                           data.n = constructor.get_n();
                                           data.role = role;
                                           data.concept_ = concept_;
                                       });
                               });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> constructor)
    {
        return generate_qualified_number_restriction(lhs, constructor);
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::QualifiedAtMostNumberRestrictionTag> constructor)
    {
        return generate_qualified_number_restriction(lhs, constructor);
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::QualifiedExactNumberRestrictionTag> constructor)
    {
        return generate_qualified_number_restriction(lhs, constructor);
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::RoleValueMapTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            false,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::RoleValueMapTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, runir::kr::dl::AgreementTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            true,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::AgreementTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs,
                              FamilyConceptView<Family, runir::kr::dl::RoleFillersTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_role(),
                              [&](auto role)
                              {
                                  return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::RoleFillersTag>>(
                                      [&](auto& data)
                                      {
                                          data.role = role;
                                          for (auto object : constructor.get_data().objects)
                                              data.objects.push_back(object);
                                      });
                              });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::ConceptTag> lhs, FamilyConceptView<Family, runir::kr::dl::OneOfTag> constructor)
    {
        return generate_nullary(lhs,
                                [&]()
                                {
                                    return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<Family, runir::kr::dl::OneOfTag>>(
                                        [&](auto& data)
                                        {
                                            for (auto object : constructor.get_data().objects)
                                                data.objects.push_back(object);
                                        });
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::AtomicStateTag<T>>>(
                                        constructor.get_data().predicate,
                                        constructor.get_data().polarity);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::AtomicGoalTag<T>>>(
                                        constructor.get_data().predicate,
                                        constructor.get_data().polarity);
                                });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::UniversalTag>)
    {
        return generate_nullary(lhs, [&] { return intern_nullary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::UniversalTag>>(); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::IntersectionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            true,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::IntersectionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::UnionTag> constructor)
    {
        return generate_binary(lhs,
                               constructor.get_lhs(),
                               constructor.get_rhs(),
                               true,
                               [&](auto child_lhs, auto child_rhs)
                               { return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::UnionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::ComplementTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg) { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::ComplementTag>>(arg); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::InverseTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg) { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::InverseTag>>(arg); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::CompositionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            false,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::CompositionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs,
                              FamilyRoleView<Family, runir::kr::dl::TransitiveClosureTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::TransitiveClosureTag>>(arg); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs,
                              FamilyRoleView<Family, runir::kr::dl::ReflexiveTransitiveClosureTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg)
                              { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::ReflexiveTransitiveClosureTag>>(arg); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::RestrictionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_lhs(),
            constructor.get_rhs(),
            false,
            [&](auto child_lhs, auto child_rhs)
            { return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::RestrictionTag>>(child_lhs, child_rhs); });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::RoleTag> lhs, FamilyRoleView<Family, runir::kr::dl::IdentityTag> constructor)
    {
        return generate_unary(lhs,
                              constructor.get_arg(),
                              [&](auto arg) { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<Family, runir::kr::dl::IdentityTag>>(arg); });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::BooleanTag> lhs,
                              FamilyBooleanView<Family, runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<Family, runir::kr::dl::AtomicStateTag<T>>>(
                                        constructor.get_data().predicate,
                                        constructor.get_data().polarity);
                                });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::BooleanTag> lhs,
                              FamilyBooleanView<Family, runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(lhs,
                                [&]
                                {
                                    return intern_predicate<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>>(
                                        constructor.get_data().predicate,
                                        constructor.get_data().polarity);
                                });
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::BooleanTag> lhs, FamilyBooleanView<Family, runir::kr::dl::NonemptyTag> constructor)
    {
        return ygg::visit(
            [&](auto arg)
            {
                return generate_unary(lhs,
                                      arg,
                                      [&](auto child)
                                      { return intern_unary<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<Family, runir::kr::dl::NonemptyTag>>(child); });
            },
            constructor.get_arg());
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::NumericalTag> lhs, FamilyNumericalView<Family, runir::kr::dl::CountTag> constructor)
    {
        return ygg::visit(
            [&](auto arg)
            {
                return generate_unary(lhs,
                                      arg,
                                      [&](auto child)
                                      { return intern_unary<runir::kr::dl::NumericalTag, runir::kr::dl::Numerical<Family, runir::kr::dl::CountTag>>(child); });
            },
            constructor.get_arg());
    }

    bool generate_constructor(FamilyNonTerminalView<Family, runir::kr::dl::NumericalTag> lhs,
                              FamilyNumericalView<Family, runir::kr::dl::DistanceTag> constructor)
    {
        return generate_ternary(lhs,
                                constructor.get_lhs(),
                                constructor.get_mid(),
                                constructor.get_rhs(),
                                [&](auto child_lhs, auto child_mid, auto child_rhs)
                                {
                                    return intern_wrapped<runir::kr::dl::NumericalTag, runir::kr::dl::Numerical<Family, runir::kr::dl::DistanceTag>>(
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
    Generator(FamilyGrammarView<Family> grammar,
              const std::vector<tyr::planning::StateView<Kind>>& states,
              runir::kr::dl::ConstructorRepositoryFor<Family>& output_repository,
              const GenerateOptions& options) :
        m_grammar(grammar),
        m_states(states),
        m_output_repository(output_repository),
        m_options(options),
        m_builder(),
        m_result(),
        m_sentences(),
        m_pruning(states, output_repository),
        m_timeout(options)
    {
    }

    GenerateResultsFor<Family> run()
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

template<runir::kr::dl::FamilyTag Family, tyr::TaskKind Kind>
GenerateResultsFor<Family> generate_impl(FamilyGrammarView<Family> grammar,
                                         const std::vector<tyr::planning::StateView<Kind>>& states,
                                         runir::kr::dl::ConstructorRepositoryFor<Family>& output_repository,
                                         const GenerateOptions& options)
{
    return Generator<Family, Kind>(grammar, states, output_repository, options).run();
}

}  // namespace
template<runir::kr::dl::FamilyTag Family, tyr::TaskKind Kind>
GenerateResultsFor<Family> generate(FamilyGrammarView<Family> grammar,
                                    const std::vector<tyr::planning::StateView<Kind>>& states,
                                    runir::kr::dl::ConstructorRepositoryFor<Family>& output_repository,
                                    const GenerateOptions& options)
{
    return generate_impl<Family, Kind>(grammar, states, output_repository, options);
}

template GenerateResultsFor<runir::kr::BaseFamilyTag>
generate<runir::kr::BaseFamilyTag, tyr::GroundTag>(FamilyGrammarView<runir::kr::BaseFamilyTag>,
                                                   const std::vector<tyr::planning::StateView<tyr::GroundTag>>&,
                                                   runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>&,
                                                   const GenerateOptions&);

template GenerateResultsFor<runir::kr::BaseFamilyTag>
generate<runir::kr::BaseFamilyTag, tyr::LiftedTag>(FamilyGrammarView<runir::kr::BaseFamilyTag>,
                                                   const std::vector<tyr::planning::StateView<tyr::LiftedTag>>&,
                                                   runir::kr::dl::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>&,
                                                   const GenerateOptions&);

}
