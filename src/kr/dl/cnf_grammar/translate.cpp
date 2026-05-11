#include "runir/kr/dl/cnf_grammar/translate.hpp"

#include "runir/kr/dl/cnf_grammar/canonicalization.hpp"

#include <cassert>
#include <concepts>
#include <string>
#include <unordered_set>

namespace runir::kr::dl::cnf_grammar
{
namespace
{

template<typename T>
auto intern(ConstructorRepository& repository, tyr::Data<T>& data)
{
    canonicalize(data);
    return repository.get_or_create(data).first;
}

template<runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(ConstructorRepository& repository, tyr::Index<T> index)
{
    tyr::Data<Constructor<Category>> data(index);
    return intern(repository, data);
}

struct TranslationContext
{
    const runir::kr::dl::grammar::GrammarView& source;
    ConstructorRepository& repository;
    tyr::Data<GrammarTag> data;
    std::unordered_set<std::string> non_terminal_names;
    size_t next_fresh_index = 0;

    TranslationContext(const runir::kr::dl::grammar::GrammarView& source_, ConstructorRepository& repository_) : source(source_), repository(repository_)
    {
        assert(repository.get_planning_repository_ptr() == source.get_context().get_planning_repository_ptr());
        data.domain = source.get_data().domain;
    }
};

template<runir::kr::dl::CategoryTag Category>
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

    auto translate_non_terminal(runir::kr::dl::grammar::NonTerminalView<Category> source)
    {
        const auto name = std::string(source.get_name().str());
        non_terminal_names().insert(name);

        tyr::Data<NonTerminal<Category>> data(name);
        return intern(repository(), data);
    }

    auto translate_to_non_terminal(runir::kr::dl::grammar::ConstructorOrNonTerminalView<Category> source)
    {
        return source.get_variant().apply(
            [&](auto value)
            {
                using View = std::decay_t<decltype(value)>;
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
            });
    }

    auto translate_constructor(runir::kr::dl::grammar::ConstructorView<Category> source)
    {
        return source.get_variant().apply([&](auto value) { return translate_concrete_constructor(value); });
    }

    void translate_rule(runir::kr::dl::grammar::DerivationRuleView<Category> source)
    {
        const auto lhs = translate_non_terminal(source.get_lhs());

        for (const auto symbol : source.get_rhs())
        {
            symbol.get_variant().apply(
                [&](auto value)
                {
                    using View = std::decay_t<decltype(value)>;
                    if constexpr (std::same_as<View, runir::kr::dl::grammar::NonTerminalView<Category>>)
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

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::BotTag>)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::BotTag>> data;
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::TopTag>)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::TopTag>> data;
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::AtomicGoalTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::IntersectionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::IntersectionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                                translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::UnionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::UnionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                         translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::NegationTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::NegationTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::ValueRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::ValueRestrictionTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index(),
                                                                    translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::ExistentialQuantificationTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::ExistentialQuantificationTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index(),
                                                                             translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::RoleValueMapContainmentTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::RoleValueMapContainmentTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index(),
                                                                           translate_child<runir::kr::dl::RoleTag>(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::RoleValueMapEqualityTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::RoleValueMapEqualityTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index(),
                                                                        translate_child<runir::kr::dl::RoleTag>(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::ConceptView<runir::kr::dl::NominalTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        tyr::Data<Concept<runir::kr::dl::NominalTag>> data(source.get_data().object);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::UniversalTag>)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::UniversalTag>> data;
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::AtomicGoalTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::IntersectionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::IntersectionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                             translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::UnionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::UnionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                      translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::ComplementTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::ComplementTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::InverseTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::InverseTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::CompositionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::CompositionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                            translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::TransitiveClosureTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::TransitiveClosureTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::ReflexiveTransitiveClosureTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::ReflexiveTransitiveClosureTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::RestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::RestrictionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                            translate_child<runir::kr::dl::ConceptTag>(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::RoleView<runir::kr::dl::IdentityTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        tyr::Data<Role<runir::kr::dl::IdentityTag>> data(translate_child<runir::kr::dl::ConceptTag>(source.get_arg()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::BooleanView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        tyr::Data<Boolean<runir::kr::dl::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::BooleanView<runir::kr::dl::NonemptyTag> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        const auto arg = source.get_arg().apply(
            [&](auto value) -> typename tyr::Data<Boolean<runir::kr::dl::NonemptyTag>>::Arg
            {
                using View = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<View, runir::kr::dl::grammar::ConstructorOrNonTerminalView<runir::kr::dl::ConceptTag>>)
                    return translate_child<runir::kr::dl::ConceptTag>(value).get_index();
                else
                    return translate_child<runir::kr::dl::RoleTag>(value).get_index();
            });

        tyr::Data<Boolean<runir::kr::dl::NonemptyTag>> data(arg);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::NumericalView<runir::kr::dl::CountTag> source)
        requires std::same_as<Category, runir::kr::dl::NumericalTag>
    {
        const auto arg = source.get_arg().apply(
            [&](auto value) -> typename tyr::Data<Numerical<runir::kr::dl::CountTag>>::Arg
            {
                using View = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<View, runir::kr::dl::grammar::ConstructorOrNonTerminalView<runir::kr::dl::ConceptTag>>)
                    return translate_child<runir::kr::dl::ConceptTag>(value).get_index();
                else
                    return translate_child<runir::kr::dl::RoleTag>(value).get_index();
            });

        tyr::Data<Numerical<runir::kr::dl::CountTag>> data(arg);
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::NumericalView<runir::kr::dl::DistanceTag> source)
        requires std::same_as<Category, runir::kr::dl::NumericalTag>
    {
        tyr::Data<Numerical<runir::kr::dl::DistanceTag>> data(translate_child<runir::kr::dl::ConceptTag>(source.get_lhs()).get_index(),
                                                              translate_child<runir::kr::dl::RoleTag>(source.get_mid()).get_index(),
                                                              translate_child<runir::kr::dl::ConceptTag>(source.get_rhs()).get_index());
        return intern_constructor<Category>(repository(), intern(repository(), data).get_index());
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
        context.data.template get_start<Category>() = translator.translate_non_terminal(*start).get_index();

    for (const auto rule : context.source.template get_derivation_rules<Category>())
        translator.translate_rule(rule);
}

}

GrammarView translate(const runir::kr::dl::grammar::GrammarView& grammar, ConstructorRepository& repository)
{
    auto context = TranslationContext(grammar, repository);

    translate_category<runir::kr::dl::ConceptTag>(context);
    translate_category<runir::kr::dl::RoleTag>(context);
    translate_category<runir::kr::dl::BooleanTag>(context);
    translate_category<runir::kr::dl::NumericalTag>(context);

    canonicalize(context.data);
    return intern(repository, context.data);
}

}
