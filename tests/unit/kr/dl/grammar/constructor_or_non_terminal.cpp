#include <concepts>
#include <runir/kr/dl/grammar/constructor_or_non_terminal_data.hpp>
#include <runir/kr/dl/grammar/constructor_or_non_terminal_index.hpp>
#include <runir/kr/dl/grammar/constructor_or_non_terminal_view.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
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
           data.value;
           data.clear();
           view.get_index();
           view.get_variant();
       };

template<typename Family, typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::grammar::ConstructorRepositoryFor<Family>> && ...);
}

template<typename Family, typename Category>
consteval bool view_alias()
{
    using Entity = kr::dl::grammar::ConstructorOrNonTerminal<Family, Category>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::grammar::ConstructorRepositoryFor<Family>>;
    static_assert(std::same_as<View, kr::dl::grammar::FamilyConstructorOrNonTerminalView<Family, Category>>);
    if constexpr (std::same_as<Family, kr::BaseFamilyTag>)
        return std::same_as<View, kr::dl::grammar::BaseConstructorOrNonTerminalView<Category>>;
    else
        return true;
}

template<typename Family, typename... Categories>
consteval bool view_aliases(ygg::TypeList<Categories...>)
{
    return (view_alias<Family, Categories>() && ...);
}

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::grammar::FamilyConstructorOrNonTerminalTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::grammar::FamilyConstructorOrNonTerminalTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::grammar::FamilyConstructorOrNonTerminalTypes<kr::UnsFamilyTag> {}));
static_assert(view_aliases<kr::BaseFamilyTag>(kr::dl::CategoryTags {}));
static_assert(view_aliases<kr::ExtFamilyTag>(kr::dl::CategoryTags {}));
static_assert(view_aliases<kr::UnsFamilyTag>(kr::dl::CategoryTags {}));

}  // namespace

}
