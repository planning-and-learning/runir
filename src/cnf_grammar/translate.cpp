#include "runir/cnf_grammar/translate.hpp"

#include "runir/cnf_grammar/canonicalization.hpp"

#include <cassert>
#include <concepts>
#include <string>
#include <unordered_set>

namespace runir::cnf_grammar
{
namespace
{

template<typename T>
auto intern(ConstructorRepository& repository, tyr::Data<T>& data)
{
    canonicalize(data);
    return repository.get_or_create(data).first;
}

template<runir::CategoryTag Category, typename T>
auto intern_constructor(ConstructorRepository& repository, tyr::Index<T> index)
{
    tyr::Data<Constructor<Category>> data(index);
    return intern(repository, data);
}

struct TranslationContext
{
    const runir::grammar::GrammarView& source;
    ConstructorRepository& repository;
    tyr::Data<GrammarTag> data;
    std::unordered_set<std::string> non_terminal_names;
    size_t next_fresh_index = 0;

    TranslationContext(const runir::grammar::GrammarView& source_, ConstructorRepository& repository_) : source(source_), repository(repository_)
    {
        assert(repository.get_planning_repository_ptr() == source.get_context().get_planning_repository_ptr());
        data.domain = source.get_data().domain;
    }
};

template<runir::CategoryTag Category>
class Translator
{
private:
    TranslationContext& m_context;

    auto& repository() const noexcept { return m_context.repository; }
    auto& data() const noexcept { return m_context.data; }
    auto& non_terminal_names() const noexcept { return m_context.non_terminal_names; }
    auto& next_fresh_index() const noexcept { return m_context.next_fresh_index; }

    auto make_fresh_non_terminal()
    {
        std::string name;
        do
        {
            name = std::string("<") + Category::name + "_" + std::to_string(next_fresh_index()++) + ">";
        } while (non_terminal_names().contains(name));

        non_terminal_names().insert(name);

        tyr::Data<NonTerminal<Category>> data(name);
        return intern(repository(), data);
    }

    void add_derivation_rule(NonTerminalView<Category> lhs, ConstructorView<Category> rhs)
    {
        tyr::Data<DerivationRule<Category>> data(lhs.get_index(), rhs.get_index());
        this->data().template get_derivation_rules<Category>().push_back(intern(repository(), data).get_index());
    }

    void add_substitution_rule(NonTerminalView<Category> lhs, NonTerminalView<Category> rhs)
    {
        tyr::Data<SubstitutionRule<Category>> data(lhs.get_index(), rhs.get_index());
        this->data().template get_substitution_rules<Category>().push_back(intern(repository(), data).get_index());
    }

public:
    Translator(TranslationContext& context) : m_context(context) {}

    auto translate_non_terminal(runir::grammar::NonTerminalView<Category> source)
    {
        const auto name = std::string(source.get_name().str());
        non_terminal_names().insert(name);

        tyr::Data<NonTerminal<Category>> data(name);
        return intern(repository(), data);
    }

    auto translate_to_non_terminal(runir::grammar::ConstructorOrNonTerminalView<Category> source)
    {
        return source.get_variant().apply(
            [&](auto value)
            {
                using View = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<View, runir::grammar::NonTerminalView<Category>>)
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
            });
    }

    auto translate_constructor(runir::grammar::ConstructorView<Category> source)
    {
        return source.get_variant().apply([&](auto value) { return translate_concrete_constructor(value); });
    }

    void translate_rule(runir::grammar::DerivationRuleView<Category> source)
    {
        const auto lhs = translate_non_terminal(source.get_lhs());

        for (const auto symbol : source.get_rhs())
        {
            symbol.get_variant().apply(
                [&](auto value)
                {
                    using View = std::decay_t<decltype(value)>;
                    if constexpr (std::same_as<View, runir::grammar::NonTerminalView<Category>>)
                    {
                        add_substitution_rule(lhs, translate_non_terminal(value));
                    }
                    else
                    {
                        add_derivation_rule(lhs, translate_constructor(value));
                    }
                });
        }
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::BotTag>)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::BotTag>> data;
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::TopTag>)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::TopTag>> data;
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::AtomicGoalTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::IntersectionTag> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::IntersectionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                        translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::UnionTag> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::UnionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                 translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::NegationTag> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::NegationTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::ValueRestrictionTag> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::ValueRestrictionTag>> data(translate_child<runir::RoleTag>(source.get_lhs()).get_index(),
                                                            translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::ExistentialQuantificationTag> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::ExistentialQuantificationTag>> data(translate_child<runir::RoleTag>(source.get_lhs()).get_index(),
                                                                     translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::RoleValueMapContainmentTag> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::RoleValueMapContainmentTag>> data(translate_child<runir::RoleTag>(source.get_lhs()).get_index(),
                                                                   translate_child<runir::RoleTag>(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::RoleValueMapEqualityTag> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::RoleValueMapEqualityTag>> data(translate_child<runir::RoleTag>(source.get_lhs()).get_index(),
                                                                translate_child<runir::RoleTag>(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::ConceptView<runir::NominalTag> source)
        requires std::same_as<Category, runir::ConceptTag>
    {
        tyr::Data<Concept<runir::NominalTag>> data(source.get_data().object);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::UniversalTag>)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::UniversalTag>> data;
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::grammar::RoleView<runir::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::grammar::RoleView<runir::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::AtomicGoalTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::IntersectionTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::IntersectionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                     translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::UnionTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::UnionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(), translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::ComplementTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::ComplementTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::InverseTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::InverseTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::CompositionTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::CompositionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                    translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::TransitiveClosureTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::TransitiveClosureTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::ReflexiveTransitiveClosureTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::ReflexiveTransitiveClosureTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::RestrictionTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::RestrictionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                    translate_child<runir::ConceptTag>(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::RoleView<runir::IdentityTag> source)
        requires std::same_as<Category, runir::RoleTag>
    {
        tyr::Data<Role<runir::IdentityTag>> data(translate_child<runir::ConceptTag>(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::grammar::BooleanView<runir::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::BooleanTag>
    {
        tyr::Data<Boolean<runir::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::BooleanView<runir::NonemptyTag> source)
        requires std::same_as<Category, runir::BooleanTag>
    {
        const auto arg = source.get_arg().apply(
            [&](auto value) -> typename tyr::Data<Boolean<runir::NonemptyTag>>::Arg
            {
                using View = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<View, runir::grammar::ConstructorOrNonTerminalView<runir::ConceptTag>>)
                    return translate_child<runir::ConceptTag>(value).get_index();
                else
                    return translate_child<runir::RoleTag>(value).get_index();
            });

        tyr::Data<Boolean<runir::NonemptyTag>> data(arg);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::NumericalView<runir::CountTag> source)
        requires std::same_as<Category, runir::NumericalTag>
    {
        const auto arg = source.get_arg().apply(
            [&](auto value) -> typename tyr::Data<Numerical<runir::CountTag>>::Arg
            {
                using View = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<View, runir::grammar::ConstructorOrNonTerminalView<runir::ConceptTag>>)
                    return translate_child<runir::ConceptTag>(value).get_index();
                else
                    return translate_child<runir::RoleTag>(value).get_index();
            });

        tyr::Data<Numerical<runir::CountTag>> data(arg);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::grammar::NumericalView<runir::DistanceTag> source)
        requires std::same_as<Category, runir::NumericalTag>
    {
        tyr::Data<Numerical<runir::DistanceTag>> data(translate_child<runir::ConceptTag>(source.get_lhs()).get_index(),
                                                      translate_child<runir::RoleTag>(source.get_mid()).get_index(),
                                                      translate_child<runir::ConceptTag>(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<runir::CategoryTag ChildCategory>
    auto translate_child(runir::grammar::ConstructorOrNonTerminalView<ChildCategory> source)
    {
        auto child_translator = Translator<ChildCategory>(m_context);
        return child_translator.translate_to_non_terminal(source);
    }
};

template<runir::CategoryTag Category>
void translate_category(TranslationContext& context)
{
    auto translator = Translator<Category>(context);

    if (auto start = context.source.template get_start<Category>())
        context.data.template get_start<Category>() = translator.translate_non_terminal(*start).get_index();

    for (const auto rule : context.source.template get_derivation_rules<Category>())
        translator.translate_rule(rule);
}

}

GrammarView translate(const runir::grammar::GrammarView& grammar, ConstructorRepository& repository)
{
    auto context = TranslationContext(grammar, repository);

    translate_category<runir::ConceptTag>(context);
    translate_category<runir::RoleTag>(context);
    translate_category<runir::BooleanTag>(context);
    translate_category<runir::NumericalTag>(context);

    canonicalize(context.data);
    return intern(repository, context.data);
}

}
