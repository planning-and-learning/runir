#include "runir/kr/dl/grammar/declarations.hpp"

#include <concepts>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/non_terminal_data.hpp>
#include <runir/kr/dl/grammar/non_terminal_view.hpp>
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
           data.name;
           data.clear();
           view.get_index();
           view.get_name();
       };

template<typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::grammar::ConstructorRepositoryFor> && ...);
}

template<typename Category>
consteval bool view_alias()
{
    using Entity = kr::dl::grammar::NonTerminal<Category>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::grammar::ConstructorRepositoryFor>;
    static_assert(std::same_as<View, kr::dl::grammar::FamilyNonTerminalView<Category>>);
    if constexpr (false)
        return std::same_as<View, kr::dl::grammar::BaseNonTerminalView<Category>>;
    else
        return true;
}

template<typename... Categories>
consteval bool view_aliases(ygg::TypeList<Categories...>)
{
    return (view_alias<Categories>() && ...);
}

static_assert(indexed_data_views<>(kr::dl::grammar::FamilyNonTerminalTypes {}));
static_assert(view_aliases<>(kr::dl::CategoryTags {}));

}  // namespace

}
