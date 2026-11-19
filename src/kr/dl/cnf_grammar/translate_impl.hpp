#ifndef RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_IMPL_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_IMPL_HPP_

#include "runir/kr/dl/cnf_grammar/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/translate.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"

#include <cassert>
#include <concepts>
#include <string>
#include <tyr/formalism/planning/repository.hpp>
#include <unordered_set>
#include <urd/repr/values/std_set.hpp>
#include <urd/store.hpp>
#include <utility>

namespace runir::kr::dl::cnf_grammar::detail
{

struct TranslationContext
{
    using GrammarBuilder = urd::builder<Grammar, ConstructorRepository>;
    using GrammarData = urd::handle<GrammarBuilder>;

    const runir::kr::dl::grammar::GrammarView& source;
    ConstructorRepository& repository;
    Builder builder;
    GrammarData data;
    urd::Store<urd::set<std::string>, urd::stable_refs> non_terminal_names;
    size_t next_fresh_index = 0;

    TranslationContext(const runir::kr::dl::grammar::GrammarView& source_, ConstructorRepository& repository_) :
        source(source_),
        repository(repository_),
        builder(),
        data(runir::kr::dl::cnf_grammar::checkout<Grammar>(builder))
    {
        // Compare raw pointers: operator== on the shared_ptrs would complete the planning repository type.
        assert(repository.get_planning_repository_ptr().get() == source.context().get_planning_repository_ptr().get());
        urd::store(data->domain, source.get_domain());
    }
};

template<runir::kr::dl::CategoryTag Category>
const char* category_prefix()
{
    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        return "c";
    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
        return "r";
    else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
        return "b";
    else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
        return "n";
}

template<runir::kr::dl::CategoryTag Category>
class Translator
{
private:
    TranslationContext* m_context;

    [[nodiscard]] auto& repository() const noexcept { return m_context->repository; }

    [[nodiscard]] auto& data() const noexcept { return *m_context->data; }

    [[nodiscard]] auto& non_terminal_names() const noexcept { return m_context->non_terminal_names; }

    [[nodiscard]] auto& next_fresh_index() const noexcept { return m_context->next_fresh_index; }

    template<typename T, typename Initialize>
    auto intern(Initialize&& initialize)
    {
        auto data = runir::kr::dl::cnf_grammar::checkout<T>(m_context->builder);
        std::forward<Initialize>(initialize)(*data);
        return repository().insert(*data, m_context->builder).first;
    }

    /// A constructor of Category, as a derivation rule's right-hand side holds it (a child of its concrete kind).
    using ConstructorChild = urd::built_t<Constructor<Category>, ConstructorRepository>;

    template<typename T, typename Initialize>
    ConstructorChild intern_concrete(Initialize&& initialize)
    { return intern<T>(std::forward<Initialize>(initialize)); }

    template<typename T>
    auto intern_nullary()
    {
        return intern_concrete<T>([](auto&) {});
    }

    template<typename T, typename Operand>
    auto intern_unary(Operand operand)
    {
        return intern_concrete<T>([&](auto& data) { data.operand = std::move(operand); });
    }

    template<typename T, typename Left, typename Right>
    auto intern_binary(Left left, Right right)
    {
        return intern_concrete<T>(
            [&](auto& data)
            {
                data.left_operand = left;
                data.right_operand = right;
            });
    }

    template<typename T, typename Role, typename Qualifier>
    auto intern_qualified(Role role, Qualifier qualifier)
    {
        return intern_concrete<T>(
            [&](auto& data)
            {
                data.role = role;
                data.qualifier = qualifier;
            });
    }

    template<typename T, typename Predicate>
    auto intern_predicate(Predicate predicate)
    {
        return intern_concrete<T>([&](auto& data) { urd::store(data.predicate, predicate); });
    }

    template<typename T, typename Predicate>
    auto intern_predicate(Predicate predicate, bool polarity)
    {
        return intern_concrete<T>(
            [&](auto& data)
            {
                urd::store(data.predicate, predicate);
                data.polarity = polarity;
            });
    }

    template<typename T, typename Role>
    auto intern_number(ygg::uint_t n, Role role)
    {
        return intern_concrete<T>(
            [&](auto& data)
            {
                data.n = n;
                data.role = role;
            });
    }

    template<typename T, typename Role, typename Qualifier>
    auto intern_qualified_number(ygg::uint_t n, Role role, Qualifier qualifier)
    {
        return intern_concrete<T>(
            [&](auto& data)
            {
                data.n = n;
                data.role = role;
                data.qualifier = qualifier;
            });
    }

    auto make_fresh_non_terminal()
    {
        auto data = runir::kr::dl::cnf_grammar::checkout<NonTerminal<Category>>(m_context->builder);
        auto name = std::string {};
        while (name.empty() || non_terminal_names().contains(name))
        {
            name = category_prefix<Category>();
            name += '_';
            name += std::to_string(next_fresh_index()++);
        }

        data->name = name;
        non_terminal_names().insert(std::move(name));

        return repository().insert(*data, m_context->builder).first;
    }

    void add_derivation_rule(NonTerminalView<Category> lhs, ConstructorChild rhs)
    {
        const auto rule = intern<DerivationRule<Category>>(
            [&](auto& data)
            {
                data.lhs = lhs;
                data.rhs = std::move(rhs);
            });
        urd::append(this->data().*derivation_rules_member<Category, TranslationContext::GrammarBuilder>(), rule);
    }

    void add_substitution_rule(NonTerminalView<Category> lhs, NonTerminalView<Category> rhs)
    {
        const auto rule = intern<SubstitutionRule<Category>>(
            [&](auto& data)
            {
                data.lhs = lhs;
                data.rhs = rhs;
            });
        urd::append(this->data().*substitution_rules_member<Category, TranslationContext::GrammarBuilder>(), rule);
    }

public:
    Translator(TranslationContext& context) : m_context(&context) {}

    auto translate_non_terminal(runir::kr::dl::grammar::NonTerminalView<Category> source)
    {
        auto data = runir::kr::dl::cnf_grammar::checkout<NonTerminal<Category>>(m_context->builder);
        data->name = source.get_name().str();
        non_terminal_names().insert(std::string(data->name.str()));

        return repository().insert(*data, m_context->builder).first;
    }

    NonTerminalView<Category> translate_to_non_terminal(runir::kr::dl::grammar::ConstructorOrNonTerminalView<Category> source)
    {
        return std::visit(
            [&]<typename View>(View value)
            {
                if constexpr (std::same_as<View, runir::kr::dl::grammar::NonTerminalView<Category>>)
                {
                    return translate_non_terminal(value);
                }
                else
                {
                    auto constructor = translate_constructor(value);
                    const auto non_terminal = make_fresh_non_terminal();
                    add_derivation_rule(non_terminal, std::move(constructor));
                    return non_terminal;
                }
            },
            source);
    }

    ConstructorChild translate_constructor(runir::kr::dl::grammar::ConstructorView<Category> source)
    {
        return std::visit([&](auto value) { return translate_concrete_constructor(value); }, source);
    }

    void translate_rule(runir::kr::dl::grammar::DerivationRuleView<Category> source)
    {
        const auto lhs = translate_non_terminal(source.get_lhs());

        for (const auto symbol : source.get_rhs())
        {
            std::visit(
                [&]<typename View>(View value)
                {
                    if constexpr (std::same_as<View, runir::kr::dl::grammar::NonTerminalView<Category>>)
                    {
                        add_substitution_rule(lhs, translate_non_terminal(value));
                    }
                    else
                    {
                        add_derivation_rule(lhs, translate_constructor(value));
                    }
                },
                symbol);
        }
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::BotTag>)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    { return intern_nullary<Concept<CnfGrammarFamily, runir::kr::dl::BotTag>>(); }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::TopTag>)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    { return intern_nullary<Concept<CnfGrammarFamily, runir::kr::dl::TopTag>>(); }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    { return intern_predicate<Concept<CnfGrammarFamily, runir::kr::dl::AtomicStateTag<T>>>(source.get_predicate()); }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    { return intern_predicate<Concept<CnfGrammarFamily, runir::kr::dl::AtomicGoalTag<T>>>(source.get_predicate(), source.get_polarity()); }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::IntersectionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_left_operand());
        const auto rhs = translate_to_non_terminal(source.get_right_operand());
        return intern_binary<Concept<CnfGrammarFamily, runir::kr::dl::IntersectionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::UnionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_left_operand());
        const auto rhs = translate_to_non_terminal(source.get_right_operand());
        return intern_binary<Concept<CnfGrammarFamily, runir::kr::dl::UnionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::NegationTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto arg = translate_to_non_terminal(source.get_operand());
        return intern_unary<Concept<CnfGrammarFamily, runir::kr::dl::NegationTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::ValueRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        const auto qualifier = translate_to_non_terminal(source.get_qualifier());
        return intern_qualified<Concept<CnfGrammarFamily, runir::kr::dl::ValueRestrictionTag>>(role, qualifier);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::ExistentialQuantificationTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        const auto qualifier = translate_to_non_terminal(source.get_qualifier());
        return intern_qualified<Concept<CnfGrammarFamily, runir::kr::dl::ExistentialQuantificationTag>>(role, qualifier);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtLeastNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        return intern_number<Concept<CnfGrammarFamily, runir::kr::dl::AtLeastNumberRestrictionTag>>(source.get_n(), role);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtMostNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        return intern_number<Concept<CnfGrammarFamily, runir::kr::dl::AtMostNumberRestrictionTag>>(source.get_n(), role);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::ExactNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        return intern_number<Concept<CnfGrammarFamily, runir::kr::dl::ExactNumberRestrictionTag>>(source.get_n(), role);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        const auto qualifier = translate_to_non_terminal(source.get_qualifier());
        return intern_qualified_number<Concept<CnfGrammarFamily, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>(source.get_n(), role, qualifier);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::QualifiedAtMostNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        const auto qualifier = translate_to_non_terminal(source.get_qualifier());
        return intern_qualified_number<Concept<CnfGrammarFamily, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>(source.get_n(), role, qualifier);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::QualifiedExactNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        const auto qualifier = translate_to_non_terminal(source.get_qualifier());
        return intern_qualified_number<Concept<CnfGrammarFamily, runir::kr::dl::QualifiedExactNumberRestrictionTag>>(source.get_n(), role, qualifier);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::RoleValueMapTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_child<runir::kr::dl::RoleTag>(source.get_left_operand());
        const auto rhs = translate_child<runir::kr::dl::RoleTag>(source.get_right_operand());
        return intern_binary<Concept<CnfGrammarFamily, runir::kr::dl::RoleValueMapTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AgreementTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_child<runir::kr::dl::RoleTag>(source.get_left_operand());
        const auto rhs = translate_child<runir::kr::dl::RoleTag>(source.get_right_operand());
        return intern_binary<Concept<CnfGrammarFamily, runir::kr::dl::AgreementTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::RoleFillersTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role());
        return intern_concrete<Concept<CnfGrammarFamily, runir::kr::dl::RoleFillersTag>>(
            [&](auto& data)
            {
                data.role = role;
                for (const auto object : source.get_objects())
                    urd::append(data.objects, object);
            });
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::OneOfTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        return intern_concrete<Concept<CnfGrammarFamily, runir::kr::dl::OneOfTag>>(
            [&](auto& data)
            {
                for (const auto object : source.get_objects())
                    urd::append(data.objects, object);
            });
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::NominalTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        return intern_concrete<Concept<CnfGrammarFamily, runir::kr::dl::NominalTag>>([&](auto& data) { urd::store(data.object, source.get_object()); });
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::UniversalTag>)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return intern_nullary<Role<CnfGrammarFamily, runir::kr::dl::UniversalTag>>(); }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return intern_predicate<Role<CnfGrammarFamily, runir::kr::dl::AtomicStateTag<T>>>(source.get_predicate()); }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    { return intern_predicate<Role<CnfGrammarFamily, runir::kr::dl::AtomicGoalTag<T>>>(source.get_predicate(), source.get_polarity()); }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::IntersectionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_left_operand());
        const auto rhs = translate_to_non_terminal(source.get_right_operand());
        return intern_binary<Role<CnfGrammarFamily, runir::kr::dl::IntersectionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::UnionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_left_operand());
        const auto rhs = translate_to_non_terminal(source.get_right_operand());
        return intern_binary<Role<CnfGrammarFamily, runir::kr::dl::UnionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::ComplementTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_to_non_terminal(source.get_operand());
        return intern_unary<Role<CnfGrammarFamily, runir::kr::dl::ComplementTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::InverseTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_to_non_terminal(source.get_operand());
        return intern_unary<Role<CnfGrammarFamily, runir::kr::dl::InverseTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::CompositionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_left_operand());
        const auto rhs = translate_to_non_terminal(source.get_right_operand());
        return intern_binary<Role<CnfGrammarFamily, runir::kr::dl::CompositionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::TransitiveClosureTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_to_non_terminal(source.get_operand());
        return intern_unary<Role<CnfGrammarFamily, runir::kr::dl::TransitiveClosureTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::ReflexiveTransitiveClosureTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_to_non_terminal(source.get_operand());
        return intern_unary<Role<CnfGrammarFamily, runir::kr::dl::ReflexiveTransitiveClosureTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::RestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto role = translate_to_non_terminal(source.get_role());
        const auto qualifier = translate_child<runir::kr::dl::ConceptTag>(source.get_qualifier());
        return intern_qualified<Role<CnfGrammarFamily, runir::kr::dl::RestrictionTag>>(role, qualifier);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::IdentityTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_child<runir::kr::dl::ConceptTag>(source.get_operand());
        return intern_unary<Role<CnfGrammarFamily, runir::kr::dl::IdentityTag>>(arg);
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::BooleanView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    { return intern_predicate<Boolean<CnfGrammarFamily, runir::kr::dl::AtomicStateTag<T>>>(source.get_predicate(), source.get_polarity()); }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::BooleanView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    { return intern_predicate<Boolean<CnfGrammarFamily, runir::kr::dl::AtomicGoalTag<T>>>(source.get_predicate(), source.get_polarity()); }

    auto translate_concrete_constructor(runir::kr::dl::grammar::BooleanView<runir::kr::dl::NonemptyTag> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        // The operand interned as the non-terminal view it is, not as a constructor child (a variant that may own a pool
        // handle), so that nothing is moved through a variant.
        return std::visit(
            [&]<typename View>(View value)
            {
                if constexpr (std::same_as<View, runir::kr::dl::grammar::ConstructorOrNonTerminalView<runir::kr::dl::ConceptTag>>)
                    return intern_unary<Boolean<CnfGrammarFamily, runir::kr::dl::NonemptyTag>>(translate_child<runir::kr::dl::ConceptTag>(value));
                else
                    return intern_unary<Boolean<CnfGrammarFamily, runir::kr::dl::NonemptyTag>>(translate_child<runir::kr::dl::RoleTag>(value));
            },
            source.get_operand());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::NumericalView<runir::kr::dl::CountTag> source)
        requires std::same_as<Category, runir::kr::dl::NumericalTag>
    {
        // The operand interned as the non-terminal view it is, not as a constructor child (a variant that may own a pool
        // handle), so that nothing is moved through a variant.
        return std::visit(
            [&]<typename View>(View value)
            {
                if constexpr (std::same_as<View, runir::kr::dl::grammar::ConstructorOrNonTerminalView<runir::kr::dl::ConceptTag>>)
                    return intern_unary<Numerical<CnfGrammarFamily, runir::kr::dl::CountTag>>(translate_child<runir::kr::dl::ConceptTag>(value));
                else
                    return intern_unary<Numerical<CnfGrammarFamily, runir::kr::dl::CountTag>>(translate_child<runir::kr::dl::RoleTag>(value));
            },
            source.get_operand());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::NumericalView<runir::kr::dl::DistanceTag> source)
        requires std::same_as<Category, runir::kr::dl::NumericalTag>
    {
        const auto sources = translate_child<runir::kr::dl::ConceptTag>(source.get_sources());
        const auto edges = translate_child<runir::kr::dl::RoleTag>(source.get_edges());
        const auto targets = translate_child<runir::kr::dl::ConceptTag>(source.get_targets());
        return intern_concrete<Numerical<CnfGrammarFamily, runir::kr::dl::DistanceTag>>(
            [&](auto& data)
            {
                data.sources = sources;
                data.edges = edges;
                data.targets = targets;
            });
    }

    template<runir::kr::dl::CategoryTag ChildCategory>
    auto translate_child(runir::kr::dl::grammar::ConstructorOrNonTerminalView<ChildCategory> source)
    {
        auto child_translator = Translator<ChildCategory>(*m_context);
        return child_translator.translate_to_non_terminal(source);
    }
};

template<runir::kr::dl::CategoryTag Category>
void translate_category(TranslationContext& context)
{
    auto translator = Translator<Category>(context);

    if (auto start = context.source.template get_start<Category>())
        urd::store((*context.data).*start_member<Category, TranslationContext::GrammarBuilder>(), translator.translate_non_terminal(*start));

    for (const auto rule : context.source.template get_derivation_rules<Category>())
        translator.translate_rule(rule);
}

inline auto translate_impl(const runir::kr::dl::grammar::GrammarView& grammar, ConstructorRepository& repository)
{
    auto context = TranslationContext(grammar, repository);

    translate_category<runir::kr::dl::ConceptTag>(context);
    translate_category<runir::kr::dl::RoleTag>(context);
    translate_category<runir::kr::dl::BooleanTag>(context);
    translate_category<runir::kr::dl::NumericalTag>(context);

    return repository.insert(*context.data, context.builder).first;
}

}  // namespace runir::kr::dl::cnf_grammar::detail

#endif
