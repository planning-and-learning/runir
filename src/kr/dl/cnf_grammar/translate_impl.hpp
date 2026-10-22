#ifndef RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_IMPL_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_IMPL_HPP_

#include "runir/kr/dl/cnf_grammar/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/translate.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"

#include <cassert>
#include <concepts>
#include <string>
#include <unordered_set>
#include <utility>

namespace runir::kr::dl::cnf_grammar
{
namespace detail
{

struct TranslationContext
{
    using GrammarData = ygg::UniqueObjectPoolPtr<ygg::Data<Grammar>>;

    const runir::kr::dl::grammar::GrammarView& source;
    ConstructorRepository& repository;
    Builder builder;
    GrammarData data;
    std::unordered_set<std::string> non_terminal_names;
    size_t next_fresh_index = 0;

    TranslationContext(const runir::kr::dl::grammar::GrammarView& source_, ConstructorRepository& repository_) :
        source(source_),
        repository(repository_),
        builder(),
        data(runir::kr::dl::cnf_grammar::checkout<Grammar>(builder))
    {
        // Compare raw pointers: operator== on the shared_ptrs would complete the planning repository type.
        assert(repository.get_planning_repository_ptr().get() == source.get_context().get_planning_repository_ptr().get());
        data->domain = source.get_data().domain;
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
    TranslationContext& m_context;

    auto& repository() const noexcept { return m_context.repository; }
    auto& data() const noexcept { return *m_context.data; }
    auto& non_terminal_names() const noexcept { return m_context.non_terminal_names; }
    auto& next_fresh_index() const noexcept { return m_context.next_fresh_index; }

    template<typename T, typename Initialize>
    auto intern(Initialize&& initialize)
    {
        auto data = runir::kr::dl::cnf_grammar::checkout<T>(m_context.builder);
        std::forward<Initialize>(initialize)(*data);
        return runir::kr::dl::cnf_grammar::insert(repository(), *data).first;
    }

    template<typename T>
    auto intern_constructor(ygg::Index<T> index)
    {
        return intern<Constructor<Category>>([&](auto& data) { data.variant = index; });
    }

    template<typename T, typename Initialize>
    auto intern_concrete(Initialize&& initialize)
    {
        return intern_constructor(intern<T>(std::forward<Initialize>(initialize)).get_index());
    }

    template<typename T>
    auto intern_nullary()
    {
        return intern_concrete<T>([](auto&) {});
    }

    template<typename T, typename Arg>
    auto intern_unary(Arg arg)
    {
        return intern_concrete<T>([&](auto& data) { data.arg = arg; });
    }

    template<typename T, typename Lhs, typename Rhs>
    auto intern_binary(Lhs lhs, Rhs rhs)
    {
        return intern_concrete<T>(
            [&](auto& data)
            {
                data.lhs = lhs;
                data.rhs = rhs;
            });
    }

    template<typename T, typename Predicate>
    auto intern_predicate(Predicate predicate, bool polarity)
    {
        return intern_concrete<T>(
            [&](auto& data)
            {
                data.predicate = predicate;
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

    template<typename T, typename Role, typename Concept>
    auto intern_qualified_number(ygg::uint_t n, Role role, Concept concept_index)
    {
        return intern_concrete<T>(
            [&](auto& data)
            {
                data.n = n;
                data.role = role;
                data.concept_ = concept_index;
            });
    }

    auto make_fresh_non_terminal()
    {
        auto data = runir::kr::dl::cnf_grammar::checkout<NonTerminal<Category>>(m_context.builder);
        auto name = std::string {};
        do
        {
            name = category_prefix<Category>();
            name += '_';
            name += std::to_string(next_fresh_index()++);
        } while (non_terminal_names().contains(name));

        data->name = name;
        non_terminal_names().insert(std::move(name));

        return runir::kr::dl::cnf_grammar::insert(repository(), *data).first;
    }

    void add_derivation_rule(NonTerminalView<Category> lhs, ConstructorView<Category> rhs)
    {
        const auto rule = intern<DerivationRule<Category>>(
            [&](auto& data)
            {
                data.lhs = lhs.get_index();
                data.rhs = rhs.get_index();
            });
        this->data().template get_derivation_rules<Category>().push_back(rule.get_index());
    }

    void add_substitution_rule(NonTerminalView<Category> lhs, NonTerminalView<Category> rhs)
    {
        const auto rule = intern<SubstitutionRule<Category>>(
            [&](auto& data)
            {
                data.lhs = lhs.get_index();
                data.rhs = rhs.get_index();
            });
        this->data().template get_substitution_rules<Category>().push_back(rule.get_index());
    }

public:
    Translator(TranslationContext& context) : m_context(context) {}

    auto translate_non_terminal(runir::kr::dl::grammar::NonTerminalView<Category> source)
    {
        auto data = runir::kr::dl::cnf_grammar::checkout<NonTerminal<Category>>(m_context.builder);
        data->name = source.get_name().str();
        non_terminal_names().insert(std::string(data->name.str()));

        return runir::kr::dl::cnf_grammar::insert(repository(), *data).first;
    }

    auto translate_to_non_terminal(runir::kr::dl::grammar::ConstructorOrNonTerminalView<Category> source)
    {
        return ygg::visit(
            [&]<typename View>(View value)
            {
                if constexpr (std::same_as<View, runir::kr::dl::grammar::NonTerminalView<Category>>)
                {
                    return translate_non_terminal(value);
                }
                else
                {
                    const auto constructor = translate_constructor(value);
                    const auto non_terminal = make_fresh_non_terminal();
                    add_derivation_rule(non_terminal, constructor);
                    return non_terminal;
                }
            },
            source.get_variant());
    }

    auto translate_constructor(runir::kr::dl::grammar::ConstructorView<Category> source)
    {
        return ygg::visit([&](auto value) { return translate_concrete_constructor(value); }, source.get_variant());
    }

    void translate_rule(runir::kr::dl::grammar::DerivationRuleView<Category> source)
    {
        const auto lhs = translate_non_terminal(source.get_lhs());

        for (const auto symbol : source.get_rhs())
        {
            ygg::visit(
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
                symbol.get_variant());
        }
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::BotTag>)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        return intern_nullary<Concept<runir::kr::dl::BotTag>>();
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::TopTag>)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        return intern_nullary<Concept<runir::kr::dl::TopTag>>();
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        return intern_predicate<Concept<runir::kr::dl::AtomicStateTag<T>>>(source.get_data().predicate, source.get_data().polarity);
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        return intern_predicate<Concept<runir::kr::dl::AtomicGoalTag<T>>>(source.get_data().predicate, source.get_data().polarity);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::IntersectionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_lhs()).get_index();
        const auto rhs = translate_to_non_terminal(source.get_rhs()).get_index();
        return intern_binary<Concept<runir::kr::dl::IntersectionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::UnionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_lhs()).get_index();
        const auto rhs = translate_to_non_terminal(source.get_rhs()).get_index();
        return intern_binary<Concept<runir::kr::dl::UnionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::NegationTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto arg = translate_to_non_terminal(source.get_arg()).get_index();
        return intern_unary<Concept<runir::kr::dl::NegationTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::ValueRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index();
        const auto rhs = translate_to_non_terminal(source.get_rhs()).get_index();
        return intern_binary<Concept<runir::kr::dl::ValueRestrictionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::ExistentialQuantificationTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index();
        const auto rhs = translate_to_non_terminal(source.get_rhs()).get_index();
        return intern_binary<Concept<runir::kr::dl::ExistentialQuantificationTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtLeastNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index();
        return intern_number<Concept<runir::kr::dl::AtLeastNumberRestrictionTag>>(source.get_n(), role);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtMostNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index();
        return intern_number<Concept<runir::kr::dl::AtMostNumberRestrictionTag>>(source.get_n(), role);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::ExactNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index();
        return intern_number<Concept<runir::kr::dl::ExactNumberRestrictionTag>>(source.get_n(), role);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index();
        const auto concept_index = translate_to_non_terminal(source.get_concept()).get_index();
        return intern_qualified_number<Concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>(source.get_n(), role, concept_index);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::QualifiedAtMostNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index();
        const auto concept_index = translate_to_non_terminal(source.get_concept()).get_index();
        return intern_qualified_number<Concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>(source.get_n(), role, concept_index);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::QualifiedExactNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index();
        const auto concept_index = translate_to_non_terminal(source.get_concept()).get_index();
        return intern_qualified_number<Concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>>(source.get_n(), role, concept_index);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::RoleValueMapTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index();
        const auto rhs = translate_child<runir::kr::dl::RoleTag>(source.get_rhs()).get_index();
        return intern_binary<Concept<runir::kr::dl::RoleValueMapTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AgreementTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto lhs = translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index();
        const auto rhs = translate_child<runir::kr::dl::RoleTag>(source.get_rhs()).get_index();
        return intern_binary<Concept<runir::kr::dl::AgreementTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::RoleFillersTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        const auto role = translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index();
        return intern_concrete<Concept<runir::kr::dl::RoleFillersTag>>(
            [&](auto& data)
            {
                data.role = role;
                for (auto object : source.get_data().objects)
                    data.objects.push_back(object);
            });
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::OneOfTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        return intern_concrete<Concept<runir::kr::dl::OneOfTag>>(
            [&](auto& data)
            {
                for (auto object : source.get_data().objects)
                    data.objects.push_back(object);
            });
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::NominalTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        return intern_concrete<Concept<runir::kr::dl::NominalTag>>([&](auto& data) { data.object = source.get_data().object; });
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::UniversalTag>)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return intern_nullary<Role<runir::kr::dl::UniversalTag>>();
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return intern_predicate<Role<runir::kr::dl::AtomicStateTag<T>>>(source.get_data().predicate, source.get_data().polarity);
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        return intern_predicate<Role<runir::kr::dl::AtomicGoalTag<T>>>(source.get_data().predicate, source.get_data().polarity);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::IntersectionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_lhs()).get_index();
        const auto rhs = translate_to_non_terminal(source.get_rhs()).get_index();
        return intern_binary<Role<runir::kr::dl::IntersectionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::UnionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_lhs()).get_index();
        const auto rhs = translate_to_non_terminal(source.get_rhs()).get_index();
        return intern_binary<Role<runir::kr::dl::UnionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::ComplementTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_to_non_terminal(source.get_arg()).get_index();
        return intern_unary<Role<runir::kr::dl::ComplementTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::InverseTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_to_non_terminal(source.get_arg()).get_index();
        return intern_unary<Role<runir::kr::dl::InverseTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::CompositionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_lhs()).get_index();
        const auto rhs = translate_to_non_terminal(source.get_rhs()).get_index();
        return intern_binary<Role<runir::kr::dl::CompositionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::TransitiveClosureTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_to_non_terminal(source.get_arg()).get_index();
        return intern_unary<Role<runir::kr::dl::TransitiveClosureTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::ReflexiveTransitiveClosureTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_to_non_terminal(source.get_arg()).get_index();
        return intern_unary<Role<runir::kr::dl::ReflexiveTransitiveClosureTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::RestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto lhs = translate_to_non_terminal(source.get_lhs()).get_index();
        const auto rhs = translate_child<runir::kr::dl::ConceptTag>(source.get_rhs()).get_index();
        return intern_binary<Role<runir::kr::dl::RestrictionTag>>(lhs, rhs);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::IdentityTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        const auto arg = translate_child<runir::kr::dl::ConceptTag>(source.get_arg()).get_index();
        return intern_unary<Role<runir::kr::dl::IdentityTag>>(arg);
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::BooleanView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        return intern_predicate<Boolean<runir::kr::dl::AtomicStateTag<T>>>(source.get_data().predicate, source.get_data().polarity);
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::BooleanView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        return intern_predicate<Boolean<runir::kr::dl::AtomicGoalTag<T>>>(source.get_data().predicate, source.get_data().polarity);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::BooleanView<runir::kr::dl::NonemptyTag> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        const auto arg = ygg::visit(
            [&]<typename View>(View value) -> typename ygg::Data<Boolean<runir::kr::dl::NonemptyTag>>::Arg
            {
                if constexpr (std::same_as<View, runir::kr::dl::grammar::ConstructorOrNonTerminalView<runir::kr::dl::ConceptTag>>)
                    return translate_child<runir::kr::dl::ConceptTag>(value).get_index();
                else
                    return translate_child<runir::kr::dl::RoleTag>(value).get_index();
            },
            source.get_arg());

        return intern_unary<Boolean<runir::kr::dl::NonemptyTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::NumericalView<runir::kr::dl::CountTag> source)
        requires std::same_as<Category, runir::kr::dl::NumericalTag>
    {
        const auto arg = ygg::visit(
            [&]<typename View>(View value) -> typename ygg::Data<Numerical<runir::kr::dl::CountTag>>::Arg
            {
                if constexpr (std::same_as<View, runir::kr::dl::grammar::ConstructorOrNonTerminalView<runir::kr::dl::ConceptTag>>)
                    return translate_child<runir::kr::dl::ConceptTag>(value).get_index();
                else
                    return translate_child<runir::kr::dl::RoleTag>(value).get_index();
            },
            source.get_arg());

        return intern_unary<Numerical<runir::kr::dl::CountTag>>(arg);
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::NumericalView<runir::kr::dl::DistanceTag> source)
        requires std::same_as<Category, runir::kr::dl::NumericalTag>
    {
        const auto lhs = translate_child<runir::kr::dl::ConceptTag>(source.get_lhs()).get_index();
        const auto mid = translate_child<runir::kr::dl::RoleTag>(source.get_mid()).get_index();
        const auto rhs = translate_child<runir::kr::dl::ConceptTag>(source.get_rhs()).get_index();
        return intern_concrete<Numerical<runir::kr::dl::DistanceTag>>(
            [&](auto& data)
            {
                data.lhs = lhs;
                data.mid = mid;
                data.rhs = rhs;
            });
    }

    template<runir::kr::dl::CategoryTag ChildCategory>
    auto translate_child(runir::kr::dl::grammar::ConstructorOrNonTerminalView<ChildCategory> source)
    {
        auto child_translator = Translator<ChildCategory>(m_context);
        return child_translator.translate_to_non_terminal(source);
    }
};

template<runir::kr::dl::CategoryTag Category>
void translate_category(TranslationContext& context)
{
    auto translator = Translator<Category>(context);

    if (auto start = context.source.template get_start<Category>())
        context.data->template get_start<Category>() = translator.translate_non_terminal(*start).get_index();

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

    return runir::kr::dl::cnf_grammar::insert(repository, *context.data).first;
}

}  // namespace detail

}  // namespace runir::kr::dl::cnf_grammar

#endif
