#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <concepts>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/substitution_rule_data.hpp>
#include <runir/kr/dl/cnf_grammar/substitution_rule_view.hpp>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView =
    std::constructible_from<urd::index<T>, ygg::uint_t> && urd::Identifiable<urd::index<T>> && urd::Identifiable<urd::data<T>>
    && urd::Identifiable<urd::index_view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
    && std::totally_ordered<urd::index_view<T, Repository>> && requires(urd::data<T>& data, const urd::index_view<T, Repository>& view) {
           data.index;
           data.lhs;
           data.rhs;
           view.index();
           view.get_lhs();
           view.get_rhs();
       };

template<typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, kr::dl::cnf_grammar::ConstructorRepository> && ...); }

template<typename Category>
consteval bool view_alias()
{
    using Entity = kr::dl::cnf_grammar::SubstitutionRule<Category>;
    using View = urd::index_view<Entity, kr::dl::cnf_grammar::ConstructorRepository>;
    static_assert(std::same_as<View, kr::dl::cnf_grammar::SubstitutionRuleView<Category>>);
    if constexpr (false)
        return std::same_as<View, kr::dl::cnf_grammar::SubstitutionRuleView<Category>>;
    else
        return true;
}

template<typename... Categories>
consteval bool view_aliases(boost::mp11::mp_list<Categories...>)
{ return (view_alias<Categories>() && ...); }

static_assert(indexed_data_views<>(kr::dl::cnf_grammar::SubstitutionRuleTypes {}));
static_assert(view_aliases<>(kr::dl::CategoryTags {}));

}  // namespace

}
