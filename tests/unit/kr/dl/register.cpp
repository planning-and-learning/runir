#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>
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
           data.name;
           data.identifier;
           data.clear();
           view.get_index();
           view.get_name();
           view.get_identifier();
       };

template<typename Repository, typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, Repository> && ...);
}

using RegisterTypes = ygg::TypeList<kr::dl::Register<kr::dl::ConceptTag>, kr::dl::Register<kr::dl::RoleTag>>;
using RegisterIdentifierTypes = ygg::TypeList<kr::dl::RegisterIdentifier<kr::dl::ConceptTag>, kr::dl::RegisterIdentifier<kr::dl::RoleTag>>;

template<typename... Ts>
consteval bool identifiers(ygg::TypeList<Ts...>)
{
    return ((std::constructible_from<Ts, ygg::uint_t> && ygg::Identifiable<Ts> && std::totally_ordered<Ts>) && ...);
}

template<typename... Categories>
consteval bool view_aliases(ygg::TypeList<Categories...>)
{
    return (std::same_as<kr::dl::RegisterView<Categories>, ygg::View<ygg::Index<kr::dl::Register<Categories>>, kr::dl::ExtConstructorRepository>> && ...);
}

static_assert(indexed_data_views<kr::dl::ExtConstructorRepository>(RegisterTypes {}));
static_assert(identifiers(RegisterIdentifierTypes {}));
static_assert(view_aliases(ygg::TypeList<kr::dl::ConceptTag, kr::dl::RoleTag> {}));

}  // namespace

TEST(RunirKrDlRegister, ExposesNameIdentifierAndBounds)
{
    namespace dl = kr::dl;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = dl::ExtConstructorRepositoryFactory().create(planning_repository);
    auto data = ygg::Data<dl::Register<dl::RoleTag>>(std::string("target"), dl::RegisterIdentifier<dl::RoleTag>(2));
    const auto reg = repository->get_or_create(data).first;

    EXPECT_EQ(reg.get_name(), "target");
    EXPECT_EQ(reg.get_identifier(), dl::RegisterIdentifier<dl::RoleTag>(2));
    EXPECT_THROW((dl::RegisterIdentifier<dl::RoleTag>(dl::num_registers)), std::out_of_range);
}

}
