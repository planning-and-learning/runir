#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <boost/mp11.hpp>
#include <concepts>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/grammar_data.hpp>
#include <runir/kr/dl/cnf_grammar/grammar_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>

namespace runir::tests::dl_cnf_grammar_grammar
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView = std::constructible_from<urd::index<T>, ygg::uint_t> && urd::Identifiable<urd::index<T>> && urd::Identifiable<urd::data<T>>
                          && urd::Identifiable<urd::index_view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
                          && std::totally_ordered<urd::index_view<T, Repository>> && requires(urd::data<T>& data, const urd::index_view<T, Repository>& view) {
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
                                 view.index();
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

template<typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, kr::dl::cnf_grammar::ConstructorRepository> && ...); }

static_assert(indexed_data_views<>(kr::dl::cnf_grammar::GrammarTypes {}));
static_assert(std::same_as<kr::dl::cnf_grammar::GrammarView, urd::index_view<kr::dl::cnf_grammar::Grammar, kr::dl::cnf_grammar::ConstructorRepository>>);
static_assert(std::same_as<kr::dl::cnf_grammar::GrammarView, urd::index_view<kr::dl::cnf_grammar::Grammar, kr::dl::cnf_grammar::ConstructorRepository>>);
static_assert(std::same_as<kr::dl::cnf_grammar::GrammarView, urd::index_view<kr::dl::cnf_grammar::Grammar, kr::dl::cnf_grammar::ConstructorRepository>>);
static_assert(std::same_as<kr::dl::cnf_grammar::GrammarView, kr::dl::cnf_grammar::GrammarView>);
static_assert(std::same_as<kr::dl::cnf_grammar::GrammarView, kr::dl::cnf_grammar::GrammarView>);
static_assert(std::same_as<kr::dl::cnf_grammar::GrammarView, kr::dl::cnf_grammar::GrammarView>);

template<typename Category>
consteval bool data_accessors()
{
    using Data = urd::data<kr::dl::cnf_grammar::Grammar>;
    using kr::dl::cnf_grammar::derivation_rules_member;
    using kr::dl::cnf_grammar::start_member;
    using kr::dl::cnf_grammar::substitution_rules_member;

    if constexpr (std::same_as<Category, kr::dl::ConceptTag>)
        return start_member<Category>() == &Data::concept_start && derivation_rules_member<Category>() == &Data::concept_derivation_rules
               && substitution_rules_member<Category>() == &Data::concept_substitution_rules;
    else if constexpr (std::same_as<Category, kr::dl::RoleTag>)
        return start_member<Category>() == &Data::role_start && derivation_rules_member<Category>() == &Data::role_derivation_rules
               && substitution_rules_member<Category>() == &Data::role_substitution_rules;
    else if constexpr (std::same_as<Category, kr::dl::BooleanTag>)
        return start_member<Category>() == &Data::boolean_start && derivation_rules_member<Category>() == &Data::boolean_derivation_rules
               && substitution_rules_member<Category>() == &Data::boolean_substitution_rules;
    else if constexpr (std::same_as<Category, kr::dl::NumericalTag>)
        return start_member<Category>() == &Data::numerical_start && derivation_rules_member<Category>() == &Data::numerical_derivation_rules
               && substitution_rules_member<Category>() == &Data::numerical_substitution_rules;
    else
        return false;
}

template<typename... Categories>
consteval bool data_accessors(boost::mp11::mp_list<Categories...>)
{ return (data_accessors<Categories>() && ...); }

static_assert(data_accessors<>(kr::dl::CategoryTags {}));

}  // namespace

}
