#include "runir/kr/dl/cnf_grammar/translate.hpp"

#include "runir/kr/dl/cnf_grammar/base/translate.hpp"
#include "runir/kr/dl/cnf_grammar/canonicalization.hpp"
#include "runir/kr/dl/cnf_grammar/ext/repository.hpp"
#include "runir/kr/dl/cnf_grammar/uns/repository.hpp"

#include <cassert>
#include <concepts>
#include <string>
#include <unordered_set>

namespace runir::kr::dl::cnf_grammar
{
namespace
{

template<runir::kr::dl::FamilyTag Family, typename T>
auto intern(ConstructorRepositoryFor<Family>& repository, ygg::Data<T>& data)
{
    canonicalize(data);
    return repository.get_or_create(data).first;
}

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename T>
auto intern_constructor(ConstructorRepositoryFor<Family>& repository, ygg::Index<T> index)
{
    ygg::Data<Constructor<Family, Category>> data(index);
    return intern(repository, data);
}

template<runir::kr::dl::FamilyTag Family>
struct TranslationContext
{
    const runir::kr::dl::grammar::base::GrammarView& source;
    ConstructorRepositoryFor<Family>& repository;
    ygg::Data<Grammar<Family>> data;
    std::unordered_set<std::string> non_terminal_names;
    size_t next_fresh_index = 0;

    TranslationContext(const runir::kr::dl::grammar::base::GrammarView& source_, ConstructorRepositoryFor<Family>& repository_) :
        source(source_),
        repository(repository_)
    {
        assert(repository.get_planning_repository_ptr() == source.get_context().get_planning_repository_ptr());
        data.domain = source.get_data().domain;
    }
};

template<runir::kr::dl::CategoryTag Category>
std::string category_prefix()
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

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
class Translator
{
private:
    TranslationContext<Family>& m_context;

    auto& repository() const noexcept { return m_context.repository; }
    auto& data() const noexcept { return m_context.data; }
    auto& non_terminal_names() const noexcept { return m_context.non_terminal_names; }
    auto& next_fresh_index() const noexcept { return m_context.next_fresh_index; }

    auto make_fresh_non_terminal()
    {
        std::string name;
        do
        {
            name = category_prefix<Category>() + "_" + std::to_string(next_fresh_index()++);
        } while (non_terminal_names().contains(name));

        non_terminal_names().insert(name);

        ygg::Data<NonTerminal<Family, Category>> data(name);
        return intern(repository(), data);
    }

    void add_derivation_rule(FamilyNonTerminalView<Family, Category> lhs, FamilyConstructorView<Family, Category> rhs)
    {
        ygg::Data<DerivationRule<Family, Category>> data(lhs.get_index(), rhs.get_index());
        this->data().template get_derivation_rules<Category>().push_back(intern(repository(), data).get_index());
    }

    void add_substitution_rule(FamilyNonTerminalView<Family, Category> lhs, FamilyNonTerminalView<Family, Category> rhs)
    {
        ygg::Data<SubstitutionRule<Family, Category>> data(lhs.get_index(), rhs.get_index());
        this->data().template get_substitution_rules<Category>().push_back(intern(repository(), data).get_index());
    }

public:
    Translator(TranslationContext<Family>& context) : m_context(context) {}

    auto translate_non_terminal(runir::kr::dl::grammar::base::NonTerminalView<Category> source)
    {
        const auto name = std::string(source.get_name().str());
        non_terminal_names().insert(name);

        ygg::Data<NonTerminal<Family, Category>> data(name);
        return intern(repository(), data);
    }

    auto translate_to_non_terminal(runir::kr::dl::grammar::base::ConstructorOrNonTerminalView<Category> source)
    {
        return ygg::visit(
            [&](auto value)
            {
                using View = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<View, runir::kr::dl::grammar::base::NonTerminalView<Category>>)
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

    auto translate_constructor(runir::kr::dl::grammar::base::ConstructorView<Category> source)
    {
        return ygg::visit([&](auto value) { return translate_concrete_constructor(value); }, source.get_variant());
    }

    void translate_rule(runir::kr::dl::grammar::base::DerivationRuleView<Category> source)
    {
        const auto lhs = translate_non_terminal(source.get_lhs());

        for (const auto symbol : source.get_rhs())
        {
            ygg::visit(
                [&](auto value)
                {
                    using View = std::decay_t<decltype(value)>;
                    if constexpr (std::same_as<View, runir::kr::dl::grammar::base::NonTerminalView<Category>>)
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

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::BotTag>)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::BotTag>> data;
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::TopTag>)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::TopTag>> data;
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::AtomicGoalTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::IntersectionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::IntersectionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                                        translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::UnionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::UnionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                                 translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::NegationTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::NegationTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::ValueRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::ValueRestrictionTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index(),
                                                                            translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::ExistentialQuantificationTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::ExistentialQuantificationTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index(),
                                                                                     translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::AtLeastNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>> data(source.get_n(),
                                                                                    translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::AtMostNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>> data(source.get_n(),
                                                                                   translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::ExactNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>> data(source.get_n(),
                                                                                  translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>> data(
            source.get_n(),
            translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index(),
            translate_to_non_terminal(source.get_concept()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::QualifiedAtMostNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>> data(
            source.get_n(),
            translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index(),
            translate_to_non_terminal(source.get_concept()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::QualifiedExactNumberRestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::QualifiedExactNumberRestrictionTag>> data(
            source.get_n(),
            translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index(),
            translate_to_non_terminal(source.get_concept()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::RoleValueMapTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::RoleValueMapTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index(),
                                                                        translate_child<runir::kr::dl::RoleTag>(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::AgreementTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::AgreementTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_lhs()).get_index(),
                                                                     translate_child<runir::kr::dl::RoleTag>(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::RoleFillersTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::RoleFillersTag>> data(translate_child<runir::kr::dl::RoleTag>(source.get_role()).get_index(),
                                                                       source.get_data().objects);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::OneOfTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::OneOfTag>> data(source.get_data().objects);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::ConceptView<runir::kr::dl::NominalTag> source)
        requires std::same_as<Category, runir::kr::dl::ConceptTag>
    {
        ygg::Data<Concept<Family, runir::kr::dl::NominalTag>> data(source.get_data().object);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::UniversalTag>)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::UniversalTag>> data;
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::AtomicGoalTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::IntersectionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::IntersectionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                                     translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::UnionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::UnionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                              translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::ComplementTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::ComplementTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::InverseTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::InverseTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::CompositionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::CompositionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                                    translate_to_non_terminal(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::TransitiveClosureTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::TransitiveClosureTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::ReflexiveTransitiveClosureTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::ReflexiveTransitiveClosureTag>> data(translate_to_non_terminal(source.get_arg()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::RestrictionTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::RestrictionTag>> data(translate_to_non_terminal(source.get_lhs()).get_index(),
                                                                    translate_child<runir::kr::dl::ConceptTag>(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::RoleView<runir::kr::dl::IdentityTag> source)
        requires std::same_as<Category, runir::kr::dl::RoleTag>
    {
        ygg::Data<Role<Family, runir::kr::dl::IdentityTag>> data(translate_child<runir::kr::dl::ConceptTag>(source.get_arg()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::base::BooleanView<runir::kr::dl::AtomicStateTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        ygg::Data<Boolean<Family, runir::kr::dl::AtomicStateTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    template<tyr::formalism::FactKind T>
    auto translate_concrete_constructor(runir::kr::dl::grammar::base::BooleanView<runir::kr::dl::AtomicGoalTag<T>> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        ygg::Data<Boolean<Family, runir::kr::dl::AtomicGoalTag<T>>> data(source.get_data().predicate, source.get_data().polarity);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::BooleanView<runir::kr::dl::NonemptyTag> source)
        requires std::same_as<Category, runir::kr::dl::BooleanTag>
    {
        const auto arg = ygg::visit(
            [&](auto value) -> typename ygg::Data<Boolean<Family, runir::kr::dl::NonemptyTag>>::Arg
            {
                using View = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<View, runir::kr::dl::grammar::base::ConstructorOrNonTerminalView<runir::kr::dl::ConceptTag>>)
                    return translate_child<runir::kr::dl::ConceptTag>(value).get_index();
                else
                    return translate_child<runir::kr::dl::RoleTag>(value).get_index();
            },
            source.get_arg());

        ygg::Data<Boolean<Family, runir::kr::dl::NonemptyTag>> data(arg);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::NumericalView<runir::kr::dl::CountTag> source)
        requires std::same_as<Category, runir::kr::dl::NumericalTag>
    {
        const auto arg = ygg::visit(
            [&](auto value) -> typename ygg::Data<Numerical<Family, runir::kr::dl::CountTag>>::Arg
            {
                using View = std::decay_t<decltype(value)>;
                if constexpr (std::same_as<View, runir::kr::dl::grammar::base::ConstructorOrNonTerminalView<runir::kr::dl::ConceptTag>>)
                    return translate_child<runir::kr::dl::ConceptTag>(value).get_index();
                else
                    return translate_child<runir::kr::dl::RoleTag>(value).get_index();
            },
            source.get_arg());

        ygg::Data<Numerical<Family, runir::kr::dl::CountTag>> data(arg);
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    auto translate_concrete_constructor(runir::kr::dl::grammar::base::NumericalView<runir::kr::dl::DistanceTag> source)
        requires std::same_as<Category, runir::kr::dl::NumericalTag>
    {
        ygg::Data<Numerical<Family, runir::kr::dl::DistanceTag>> data(translate_child<runir::kr::dl::ConceptTag>(source.get_lhs()).get_index(),
                                                                      translate_child<runir::kr::dl::RoleTag>(source.get_mid()).get_index(),
                                                                      translate_child<runir::kr::dl::ConceptTag>(source.get_rhs()).get_index());
        return intern_constructor<Family, Category>(repository(), intern(repository(), data).get_index());
    }

    template<runir::kr::dl::CategoryTag ChildCategory>
    auto translate_child(runir::kr::dl::grammar::base::ConstructorOrNonTerminalView<ChildCategory> source)
    {
        auto child_translator = Translator<Family, ChildCategory>(m_context);
        return child_translator.translate_to_non_terminal(source);
    }
};

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
void translate_category(TranslationContext<Family>& context)
{
    auto translator = Translator<Family, Category>(context);

    if (auto start = context.source.template get_start<Category>())
        context.data.template get_start<Category>() = translator.translate_non_terminal(*start).get_index();

    for (const auto rule : context.source.template get_derivation_rules<Category>())
        translator.translate_rule(rule);
}

template<runir::kr::dl::FamilyTag Family>
auto translate_impl(const runir::kr::dl::grammar::base::GrammarView& grammar, ConstructorRepositoryFor<Family>& repository)
{
    auto context = TranslationContext<Family>(grammar, repository);

    translate_category<Family, runir::kr::dl::ConceptTag>(context);
    translate_category<Family, runir::kr::dl::RoleTag>(context);
    translate_category<Family, runir::kr::dl::BooleanTag>(context);
    translate_category<Family, runir::kr::dl::NumericalTag>(context);

    canonicalize(context.data);
    return intern(repository, context.data);
}

}  // namespace

template<runir::kr::dl::FamilyTag Family>
FamilyGrammarView<Family> translate(const runir::kr::dl::grammar::base::GrammarView& grammar, ConstructorRepositoryFor<Family>& repository)
{
    return translate_impl<Family>(grammar, repository);
}

template FamilyGrammarView<runir::kr::BaseFamilyTag> translate<runir::kr::BaseFamilyTag>(const runir::kr::dl::grammar::base::GrammarView&,
                                                                                         ConstructorRepositoryFor<runir::kr::BaseFamilyTag>&);

template FamilyGrammarView<runir::kr::ExtFamilyTag> translate<runir::kr::ExtFamilyTag>(const runir::kr::dl::grammar::base::GrammarView&,
                                                                                       ConstructorRepositoryFor<runir::kr::ExtFamilyTag>&);

template FamilyGrammarView<runir::kr::UnsFamilyTag> translate<runir::kr::UnsFamilyTag>(const runir::kr::dl::grammar::base::GrammarView&,
                                                                                       ConstructorRepositoryFor<runir::kr::UnsFamilyTag>&);

namespace base
{

GrammarView translate(const runir::kr::dl::grammar::base::GrammarView& grammar, ConstructorRepository& repository)
{
    return runir::kr::dl::cnf_grammar::translate<runir::kr::BaseFamilyTag>(grammar, repository);
}

}  // namespace base

}  // namespace runir::kr::dl::cnf_grammar
