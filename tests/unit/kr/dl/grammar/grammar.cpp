#include "runir/kr/dl/grammar/declarations.hpp"

#include <boost/mp11.hpp>
#include <concepts>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/grammar_data.hpp>
#include <runir/kr/dl/grammar/grammar_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
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
                                 view.get_domain();
                             };

template<typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, kr::dl::grammar::ConstructorRepository> && ...); }

static_assert(indexed_data_views<>(kr::dl::grammar::GrammarTypes {}));
static_assert(std::same_as<kr::dl::grammar::GrammarView, urd::index_view<kr::dl::grammar::GrammarTag, kr::dl::grammar::ConstructorRepository>>);
static_assert(std::same_as<kr::dl::grammar::GrammarView, urd::index_view<kr::dl::grammar::GrammarTag, kr::dl::grammar::ConstructorRepository>>);
static_assert(std::same_as<kr::dl::grammar::GrammarView, urd::index_view<kr::dl::grammar::GrammarTag, kr::dl::grammar::ConstructorRepository>>);
static_assert(std::same_as<kr::dl::grammar::GrammarView, kr::dl::grammar::GrammarView>);
static_assert(std::same_as<kr::dl::grammar::GrammarView, kr::dl::grammar::GrammarView>);
static_assert(std::same_as<kr::dl::grammar::GrammarView, kr::dl::grammar::GrammarView>);

}  // namespace

}
