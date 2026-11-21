#ifndef RUNIR_KR_DL_CNF_GRAMMAR_GENERATOR_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_GENERATOR_HPP_

#include "generation_state.hpp"
#include "pruning.hpp"
#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/detail/repository_instantiations.hpp"
#include "runir/kr/dl/cnf_grammar/generate.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <chrono>
#include <concepts>
#include <cstddef>
#include <tyr/formalism/declarations.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>

namespace runir::kr::dl::cnf_grammar::detail
{

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
        const auto concrete = m_output_repository->insert(*data, m_builder).first;

        auto wrapper = runir::kr::dl::checkout<runir::kr::dl::Constructor<Category>>(m_builder);
        urd::store(wrapper->variant, concrete);
        return m_output_repository->insert(*wrapper, m_builder).first;
    }

    template<runir::kr::dl::CategoryTag Category, typename T>
    auto intern_nullary()
    {
        return intern_wrapped<Category, T>([](auto&) {});
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Operand>
    auto intern_unary(Operand operand)
    {
        return intern_wrapped<Category, T>([&](auto& data) { urd::store(data.operand, m_output_repository->view(operand)); });
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Left, typename Right>
    auto intern_binary(Left left, Right right)
    {
        return intern_wrapped<Category, T>(
            [&](auto& data)
            {
                urd::store(data.left_operand, m_output_repository->view(left));
                urd::store(data.right_operand, m_output_repository->view(right));
            });
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Role, typename Qualifier>
    auto intern_qualified(Role role, Qualifier qualifier)
    {
        return intern_wrapped<Category, T>(
            [&](auto& data)
            {
                urd::store(data.role, m_output_repository->view(role));
                urd::store(data.qualifier, m_output_repository->view(qualifier));
            });
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Predicate>
    auto intern_predicate(Predicate predicate)
    {
        return intern_wrapped<Category, T>([&](auto& data) { urd::store(data.predicate, predicate); });
    }

    template<runir::kr::dl::CategoryTag Category, typename T, typename Predicate>
    auto intern_predicate(Predicate predicate, bool polarity)
    {
        return intern_wrapped<Category, T>(
            [&](auto& data)
            {
                urd::store(data.predicate, predicate);
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
        return generate_nullary(
            lhs,
            [&]
            {
                return intern_predicate<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicStateTag<T>>>(
                    constructor.get_predicate());
            });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(
            lhs,
            [&]
            {
                return intern_predicate<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicGoalTag<T>>>(
                    constructor.get_predicate(),
                    constructor.get_polarity());
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::BotTag>)
    {
        return generate_nullary(
            lhs,
            [&] { return intern_nullary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::BotTag>>(); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::TopTag>)
    {
        return generate_nullary(
            lhs,
            [&] { return intern_nullary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::TopTag>>(); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::NominalTag> constructor)
    {
        return generate_nullary(
            lhs,
            [&]
            {
                return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::NominalTag>>(
                    [&](auto& data) { urd::store(data.object, constructor.get_object()); });
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::IntersectionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_left_operand(),
            constructor.get_right_operand(),
            true,
            [&](auto child_lhs, auto child_rhs)
            {
                return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::IntersectionTag>>(
                    child_lhs,
                    child_rhs);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::UnionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_left_operand(),
            constructor.get_right_operand(),
            true,
            [&](auto child_lhs, auto child_rhs)
            {
                return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::UnionTag>>(child_lhs,
                                                                                                                                                child_rhs);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::NegationTag> constructor)
    {
        return generate_unary(
            lhs,
            constructor.get_operand(),
            [&](auto arg)
            { return intern_unary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::NegationTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::ValueRestrictionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_role(),
            constructor.get_qualifier(),
            false,
            [&](auto role, auto qualifier)
            {
                return intern_qualified<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::ValueRestrictionTag>>(
                    role,
                    qualifier);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::ExistentialQuantificationTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_role(),
            constructor.get_qualifier(),
            false,
            [&](auto role, auto qualifier)
            {
                return intern_qualified<runir::kr::dl::ConceptTag,
                                        runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::ExistentialQuantificationTag>>(role, qualifier);
            });
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
                                          urd::store(data.role, m_output_repository->view(role));
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
                               constructor.get_qualifier(),
                               false,
                               [&](auto role, auto qualifier)
                               {
                                   return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, Tag>>(
                                       [&](auto& data)
                                       {
                                           data.n = constructor.get_n();
                                           urd::store(data.role, m_output_repository->view(role));
                                           urd::store(data.qualifier, m_output_repository->view(qualifier));
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
            constructor.get_left_operand(),
            constructor.get_right_operand(),
            false,
            [&](auto child_lhs, auto child_rhs)
            {
                return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::RoleValueMapTag>>(
                    child_lhs,
                    child_rhs);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::AgreementTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_left_operand(),
            constructor.get_right_operand(),
            true,
            [&](auto child_lhs, auto child_rhs)
            {
                return intern_binary<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::AgreementTag>>(child_lhs,
                                                                                                                                                    child_rhs);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::RoleFillersTag> constructor)
    {
        return generate_unary(
            lhs,
            constructor.get_role(),
            [&](auto role)
            {
                return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::RoleFillersTag>>(
                    [&](auto& data)
                    {
                        urd::store(data.role, m_output_repository->view(role));
                        for (const auto object : constructor.get_objects())
                            urd::append(data.objects, object);
                    });
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::ConceptTag> lhs, ConceptView<runir::kr::dl::OneOfTag> constructor)
    {
        return generate_nullary(
            lhs,
            [&]()
            {
                return intern_wrapped<runir::kr::dl::ConceptTag, runir::kr::dl::Concept<runir::kr::dl::SemanticFamily, runir::kr::dl::OneOfTag>>(
                    [&](auto& data)
                    {
                        for (const auto object : constructor.get_objects())
                            urd::append(data.objects, object);
                    });
            });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(
            lhs,
            [&]
            {
                return intern_predicate<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicStateTag<T>>>(
                    constructor.get_predicate());
            });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(
            lhs,
            [&]
            {
                return intern_predicate<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicGoalTag<T>>>(
                    constructor.get_predicate(),
                    constructor.get_polarity());
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::UniversalTag>)
    {
        return generate_nullary(
            lhs,
            [&] { return intern_nullary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::UniversalTag>>(); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::IntersectionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_left_operand(),
            constructor.get_right_operand(),
            true,
            [&](auto child_lhs, auto child_rhs)
            {
                return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::IntersectionTag>>(child_lhs,
                                                                                                                                                 child_rhs);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::UnionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_left_operand(),
            constructor.get_right_operand(),
            true,
            [&](auto child_lhs, auto child_rhs)
            {
                return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::UnionTag>>(child_lhs, child_rhs);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::ComplementTag> constructor)
    {
        return generate_unary(
            lhs,
            constructor.get_operand(),
            [&](auto arg)
            { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::ComplementTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::InverseTag> constructor)
    {
        return generate_unary(
            lhs,
            constructor.get_operand(),
            [&](auto arg) { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::InverseTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::CompositionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_left_operand(),
            constructor.get_right_operand(),
            false,
            [&](auto child_lhs, auto child_rhs)
            {
                return intern_binary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::CompositionTag>>(child_lhs,
                                                                                                                                                child_rhs);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::TransitiveClosureTag> constructor)
    {
        return generate_unary(
            lhs,
            constructor.get_operand(),
            [&](auto arg)
            { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::TransitiveClosureTag>>(arg); });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::ReflexiveTransitiveClosureTag> constructor)
    {
        return generate_unary(
            lhs,
            constructor.get_operand(),
            [&](auto arg)
            {
                return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::ReflexiveTransitiveClosureTag>>(
                    arg);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::RestrictionTag> constructor)
    {
        return generate_binary(
            lhs,
            constructor.get_role(),
            constructor.get_qualifier(),
            false,
            [&](auto role, auto qualifier)
            {
                return intern_qualified<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::RestrictionTag>>(role,
                                                                                                                                                   qualifier);
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::RoleTag> lhs, RoleView<runir::kr::dl::IdentityTag> constructor)
    {
        return generate_unary(
            lhs,
            constructor.get_operand(),
            [&](auto arg)
            { return intern_unary<runir::kr::dl::RoleTag, runir::kr::dl::Role<runir::kr::dl::SemanticFamily, runir::kr::dl::IdentityTag>>(arg); });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::BooleanTag> lhs, BooleanView<runir::kr::dl::AtomicStateTag<T>> constructor)
    {
        return generate_nullary(
            lhs,
            [&]
            {
                return intern_predicate<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicStateTag<T>>>(
                    constructor.get_predicate(),
                    constructor.get_polarity());
            });
    }

    template<tyr::formalism::FactKind T>
    bool generate_constructor(NonTerminalView<runir::kr::dl::BooleanTag> lhs, BooleanView<runir::kr::dl::AtomicGoalTag<T>> constructor)
    {
        return generate_nullary(
            lhs,
            [&]
            {
                return intern_predicate<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, runir::kr::dl::AtomicGoalTag<T>>>(
                    constructor.get_predicate(),
                    constructor.get_polarity());
            });
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::BooleanTag> lhs, BooleanView<runir::kr::dl::NonemptyTag> constructor)
    {
        return std::visit(
            [&](auto arg)
            {
                return generate_unary(
                    lhs,
                    arg,
                    [&](auto child)
                    {
                        return intern_unary<runir::kr::dl::BooleanTag, runir::kr::dl::Boolean<runir::kr::dl::SemanticFamily, runir::kr::dl::NonemptyTag>>(
                            child);
                    });
            },
            constructor.get_operand());
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::NumericalTag> lhs, NumericalView<runir::kr::dl::CountTag> constructor)
    {
        return std::visit(
            [&](auto arg)
            {
                return generate_unary(
                    lhs,
                    arg,
                    [&](auto child)
                    {
                        return intern_unary<runir::kr::dl::NumericalTag, runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::CountTag>>(
                            child);
                    });
            },
            constructor.get_operand());
    }

    bool generate_constructor(NonTerminalView<runir::kr::dl::NumericalTag> lhs, NumericalView<runir::kr::dl::DistanceTag> constructor)
    {
        return generate_ternary(
            lhs,
            constructor.get_sources(),
            constructor.get_edges(),
            constructor.get_targets(),
            [&](auto sources, auto edges, auto targets)
            {
                return intern_wrapped<runir::kr::dl::NumericalTag, runir::kr::dl::Numerical<runir::kr::dl::SemanticFamily, runir::kr::dl::DistanceTag>>(
                    [&](auto& data)
                    {
                        urd::store(data.sources, m_output_repository->view(sources));
                        urd::store(data.edges, m_output_repository->view(edges));
                        urd::store(data.targets, m_output_repository->view(targets));
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

}

#endif
