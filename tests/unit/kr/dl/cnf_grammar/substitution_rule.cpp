#include "runir/kr/dl/cnf_grammar/declarations.hpp"

#include <concepts>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/substitution_rule_data.hpp>
#include <runir/kr/dl/cnf_grammar/substitution_rule_view.hpp>
#include <yggdrasil/core/concepts.hpp>
#include <yggdrasil/core/types.hpp>

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
           data.lhs;
           data.rhs;
           data.clear();
           view.get_index();
           view.get_lhs();
           view.get_rhs();
       };

template<typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::cnf_grammar::ConstructorRepositoryFor> && ...);
}

template<typename Category>
consteval bool view_alias()
{
    using Entity = kr::dl::cnf_grammar::SubstitutionRule<Category>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::cnf_grammar::ConstructorRepositoryFor>;
    static_assert(std::same_as<View, kr::dl::cnf_grammar::FamilySubstitutionRuleView<Category>>);
    if constexpr (false)
        return std::same_as<View, kr::dl::cnf_grammar::BaseSubstitutionRuleView<Category>>;
    else
        return true;
}

template<typename... Categories>
consteval bool view_aliases(ygg::TypeList<Categories...>)
{
    return (view_alias<Categories>() && ...);
}

static_assert(indexed_data_views<>(kr::dl::cnf_grammar::FamilySubstitutionRuleTypes {}));
static_assert(view_aliases<>(kr::dl::CategoryTags {}));

}  // namespace

}
