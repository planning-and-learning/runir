#include <concepts>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/rule_variant_data.hpp>
#include <runir/kr/ps/ext/rule_variant_index.hpp>
#include <runir/kr/ps/ext/rule_variant_view.hpp>

namespace runir::tests
{

using Entity = kr::ps::Rule<kr::ExtFamilyTag>;
using Index = ygg::Index<Entity>;
using Data = ygg::Data<Entity>;
using View = ygg::View<Index, kr::ps::ext::Repository>;

static_assert(std::constructible_from<Index, ygg::uint_t>);
static_assert(std::totally_ordered<Index>);
static_assert(std::totally_ordered<Data>);
static_assert(std::totally_ordered<View>);
static_assert(std::same_as<View, kr::ps::ext::RuleVariantView>);
static_assert(requires(Data& data) {
    data.index;
    data.symbol;
    data.variant;
    data.clear();
});
static_assert(requires(const View& view) {
    view.get_index();
    view.get_symbol();
    view.get_variant();
});

static_assert(requires(Data& data) {
    canonicalize(data);
    { is_canonical(data) } -> std::same_as<bool>;
});
static_assert(requires(kr::ps::ext::Repository& repository, ygg::Data<kr::ps::Rule<kr::BaseFamilyTag>>& data) {
    canonicalize(data);
    { is_canonical(data) } -> std::same_as<bool>;
    kr::ps::ext::get_or_create(repository, data);
});

}
