#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::tests::dl_register
{

namespace
{

template<typename T, typename Repository>
concept IndexedDataView = std::constructible_from<urd::index<T>, ygg::uint_t> && urd::Identifiable<urd::index<T>> && urd::Identifiable<urd::data<T>>
                          && urd::Identifiable<urd::index_view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
                          && std::totally_ordered<urd::index_view<T, Repository>> && requires(urd::data<T>& data, const urd::index_view<T, Repository>& view) {
                                 data.index;
                                 data.name;
                                 data.identifier;
                                 view.index();
                                 view.get_name();
                                 view.get_identifier();
                             };

template<typename Repository, typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, Repository> && ...); }

using RegisterTypes = boost::mp11::mp_list<kr::dl::Register<kr::dl::ConceptTag>, kr::dl::Register<kr::dl::RoleTag>>;
using RegisterIdentifierTypes = boost::mp11::mp_list<kr::dl::RegisterIdentifier<kr::dl::ConceptTag>, kr::dl::RegisterIdentifier<kr::dl::RoleTag>>;

template<typename... Ts>
consteval bool identifiers(boost::mp11::mp_list<Ts...>)
{ return ((std::constructible_from<Ts, ygg::uint_t> && urd::Identifiable<Ts> && std::totally_ordered<Ts>) && ...); }

template<typename... Categories>
consteval bool view_aliases(boost::mp11::mp_list<Categories...>)
{ return (std::same_as<kr::dl::RegisterView<Categories>, urd::index_view<kr::dl::Register<Categories>, kr::dl::ConstructorRepository>> && ...); }

static_assert(indexed_data_views<kr::dl::ConstructorRepository>(RegisterTypes {}));
static_assert(identifiers(RegisterIdentifierTypes {}));
static_assert(view_aliases(boost::mp11::mp_list<kr::dl::ConceptTag, kr::dl::RoleTag> {}));

}  // namespace

TEST(RunirKrDlRegister, ExposesNameAndUnboundedIdentifier)
{
    auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);
    auto data = urd::data<kr::dl::Register<kr::dl::RoleTag>>(std::string("target"), kr::dl::RegisterIdentifier<kr::dl::RoleTag>(1024));
    const auto reg = repository->insert(data).first;

    EXPECT_EQ(reg.get_name(), "target");
    EXPECT_EQ(reg.get_identifier(), kr::dl::RegisterIdentifier<kr::dl::RoleTag>(1024));
}
}
