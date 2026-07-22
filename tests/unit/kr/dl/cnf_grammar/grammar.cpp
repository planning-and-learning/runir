#include <concepts>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/grammar_data.hpp>
#include <runir/kr/dl/cnf_grammar/grammar_index.hpp>
#include <runir/kr/dl/cnf_grammar/grammar_view.hpp>
#include <utility>
#include <yggdrasil/core/concepts.hpp>

namespace runir::tests
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView =
    std::constructible_from<ygg::Index<T>, ygg::uint_t> && ygg::Identifiable<ygg::Index<T>> && ygg::Identifiable<ygg::Data<T>>
    && ygg::Identifiable<ygg::View<ygg::Index<T>, Repository>> && std::totally_ordered<ygg::Index<T>> && std::totally_ordered<ygg::Data<T>>
    && std::totally_ordered<ygg::View<ygg::Index<T>, Repository>> && requires(ygg::Data<T>& data, const ygg::View<ygg::Index<T>, Repository>& view) {
           data.index;
           data.concept_start;
           data.role_start;
           data.boolean_start;
           data.numerical_start;
           data.concept_derivation_rules;
           data.role_derivation_rules;
           data.boolean_derivation_rules;
           data.numerical_derivation_rules;
           data.concept_substitution_rules;
           data.role_substitution_rules;
           data.boolean_substitution_rules;
           data.numerical_substitution_rules;
           data.domain;
           data.clear();
           view.get_index();
           view.template get_start<kr::dl::ConceptTag>();
           view.template get_start<kr::dl::RoleTag>();
           view.template get_start<kr::dl::BooleanTag>();
           view.template get_start<kr::dl::NumericalTag>();
           view.template get_derivation_rules<kr::dl::ConceptTag>();
           view.template get_derivation_rules<kr::dl::RoleTag>();
           view.template get_derivation_rules<kr::dl::BooleanTag>();
           view.template get_derivation_rules<kr::dl::NumericalTag>();
           view.template get_substitution_rules<kr::dl::ConceptTag>();
           view.template get_substitution_rules<kr::dl::RoleTag>();
           view.template get_substitution_rules<kr::dl::BooleanTag>();
           view.template get_substitution_rules<kr::dl::NumericalTag>();
           view.get_domain();
       };

template<typename Family, typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>> && ...);
}

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::cnf_grammar::FamilyGrammarTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::cnf_grammar::FamilyGrammarTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::cnf_grammar::FamilyGrammarTypes<kr::UnsFamilyTag> {}));
static_assert(std::same_as<kr::dl::cnf_grammar::FamilyGrammarView<kr::BaseFamilyTag>,
                           ygg::View<ygg::Index<kr::dl::cnf_grammar::Grammar<kr::BaseFamilyTag>>, kr::dl::cnf_grammar::BaseConstructorRepository>>);
static_assert(std::same_as<kr::dl::cnf_grammar::FamilyGrammarView<kr::ExtFamilyTag>,
                           ygg::View<ygg::Index<kr::dl::cnf_grammar::Grammar<kr::ExtFamilyTag>>, kr::dl::cnf_grammar::ExtConstructorRepository>>);
static_assert(std::same_as<kr::dl::cnf_grammar::FamilyGrammarView<kr::UnsFamilyTag>,
                           ygg::View<ygg::Index<kr::dl::cnf_grammar::Grammar<kr::UnsFamilyTag>>, kr::dl::cnf_grammar::UnsConstructorRepository>>);
static_assert(std::same_as<kr::dl::cnf_grammar::FamilyGrammarView<kr::BaseFamilyTag>, kr::dl::cnf_grammar::BaseGrammarView>);
static_assert(std::same_as<kr::dl::cnf_grammar::FamilyGrammarView<kr::ExtFamilyTag>, kr::dl::cnf_grammar::ExtGrammarView>);
static_assert(std::same_as<kr::dl::cnf_grammar::FamilyGrammarView<kr::UnsFamilyTag>, kr::dl::cnf_grammar::UnsGrammarView>);

template<typename Family, typename Category>
consteval bool data_accessors()
{
    using Data = ygg::Data<kr::dl::cnf_grammar::Grammar<Family>>;
    using Mutable = Data&;
    using Const = const Data&;

    if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
        return std::same_as<decltype(std::declval<Mutable>().template get_start<Category>()), decltype((std::declval<Mutable>().concept_start))>
               && std::same_as<decltype(std::declval<Const>().template get_start<Category>()), decltype((std::declval<Const>().concept_start))>
               && std::same_as<decltype(std::declval<Mutable>().template get_derivation_rules<Category>()),
                               decltype((std::declval<Mutable>().concept_derivation_rules))>
               && std::same_as<decltype(std::declval<Const>().template get_derivation_rules<Category>()),
                               decltype((std::declval<Const>().concept_derivation_rules))>
               && std::same_as<decltype(std::declval<Mutable>().template get_substitution_rules<Category>()),
                               decltype((std::declval<Mutable>().concept_substitution_rules))>
               && std::same_as<decltype(std::declval<Const>().template get_substitution_rules<Category>()),
                               decltype((std::declval<Const>().concept_substitution_rules))>;
    else if constexpr (std::same_as<Category, kr::dl::RoleTag>)
        return std::same_as<decltype(std::declval<Mutable>().template get_start<Category>()), decltype((std::declval<Mutable>().role_start))>
               && std::same_as<decltype(std::declval<Const>().template get_start<Category>()), decltype((std::declval<Const>().role_start))>
               && std::same_as<decltype(std::declval<Mutable>().template get_derivation_rules<Category>()),
                               decltype((std::declval<Mutable>().role_derivation_rules))>
               && std::same_as<decltype(std::declval<Const>().template get_derivation_rules<Category>()),
                               decltype((std::declval<Const>().role_derivation_rules))>
               && std::same_as<decltype(std::declval<Mutable>().template get_substitution_rules<Category>()),
                               decltype((std::declval<Mutable>().role_substitution_rules))>
               && std::same_as<decltype(std::declval<Const>().template get_substitution_rules<Category>()),
                               decltype((std::declval<Const>().role_substitution_rules))>;
    else if constexpr (std::same_as<Category, kr::dl::BooleanTag>)
        return std::same_as<decltype(std::declval<Mutable>().template get_start<Category>()), decltype((std::declval<Mutable>().boolean_start))>
               && std::same_as<decltype(std::declval<Const>().template get_start<Category>()), decltype((std::declval<Const>().boolean_start))>
               && std::same_as<decltype(std::declval<Mutable>().template get_derivation_rules<Category>()),
                               decltype((std::declval<Mutable>().boolean_derivation_rules))>
               && std::same_as<decltype(std::declval<Const>().template get_derivation_rules<Category>()),
                               decltype((std::declval<Const>().boolean_derivation_rules))>
               && std::same_as<decltype(std::declval<Mutable>().template get_substitution_rules<Category>()),
                               decltype((std::declval<Mutable>().boolean_substitution_rules))>
               && std::same_as<decltype(std::declval<Const>().template get_substitution_rules<Category>()),
                               decltype((std::declval<Const>().boolean_substitution_rules))>;
    else if constexpr (std::same_as<Category, kr::dl::NumericalTag>)
        return std::same_as<decltype(std::declval<Mutable>().template get_start<Category>()), decltype((std::declval<Mutable>().numerical_start))>
               && std::same_as<decltype(std::declval<Const>().template get_start<Category>()), decltype((std::declval<Const>().numerical_start))>
               && std::same_as<decltype(std::declval<Mutable>().template get_derivation_rules<Category>()),
                               decltype((std::declval<Mutable>().numerical_derivation_rules))>
               && std::same_as<decltype(std::declval<Const>().template get_derivation_rules<Category>()),
                               decltype((std::declval<Const>().numerical_derivation_rules))>
               && std::same_as<decltype(std::declval<Mutable>().template get_substitution_rules<Category>()),
                               decltype((std::declval<Mutable>().numerical_substitution_rules))>
               && std::same_as<decltype(std::declval<Const>().template get_substitution_rules<Category>()),
                               decltype((std::declval<Const>().numerical_substitution_rules))>;
    else
        return false;
}

template<typename Family, typename... Categories>
consteval bool data_accessors(ygg::TypeList<Categories...>)
{
    return (data_accessors<Family, Categories>() && ...);
}

static_assert(data_accessors<kr::BaseFamilyTag>(kr::dl::CategoryTags {}));
static_assert(data_accessors<kr::ExtFamilyTag>(kr::dl::CategoryTags {}));
static_assert(data_accessors<kr::UnsFamilyTag>(kr::dl::CategoryTags {}));

}  // namespace

}
