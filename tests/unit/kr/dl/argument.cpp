#include <concepts>
#include <gtest/gtest.h>
#include <runir/kr/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::tests::dl_argument
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

using ArgumentTypes = boost::mp11::mp_list<kr::dl::Argument<kr::dl::ConceptTag>,
                                           kr::dl::Argument<kr::dl::RoleTag>,
                                           kr::dl::Argument<kr::dl::BooleanTag>,
                                           kr::dl::Argument<kr::dl::NumericalTag>>;
using ArgumentIdentifierTypes = boost::mp11::mp_list<kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>,
                                                     kr::dl::ArgumentIdentifier<kr::dl::RoleTag>,
                                                     kr::dl::ArgumentIdentifier<kr::dl::BooleanTag>,
                                                     kr::dl::ArgumentIdentifier<kr::dl::NumericalTag>>;

template<typename... Ts>
consteval bool identifiers(boost::mp11::mp_list<Ts...>)
{ return ((std::constructible_from<Ts, ygg::uint_t> && urd::Identifiable<Ts> && std::totally_ordered<Ts>) && ...); }

template<typename... Categories>
consteval bool view_aliases(boost::mp11::mp_list<Categories...>)
{ return (std::same_as<kr::dl::ArgumentView<Categories>, urd::index_view<kr::dl::Argument<Categories>, kr::dl::ConstructorRepository>> && ...); }

static_assert(indexed_data_views<kr::dl::ConstructorRepository>(ArgumentTypes {}));
static_assert(identifiers(ArgumentIdentifierTypes {}));
static_assert(view_aliases(kr::dl::CategoryTags {}));

}  // namespace

TEST(RunirKrDlArgument, ExposesNameAndIdentifier)
{
    auto planning_repository = std::make_shared<tyr::formalism::planning::Repository>();
    auto repository = kr::dl::ConstructorRepositoryFactory().create(planning_repository);
    auto data = urd::data<kr::dl::Argument<kr::dl::ConceptTag>>(std::string("source"), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(3));
    const auto argument = repository->insert(data).first;

    EXPECT_EQ(argument.get_name(), "source");
    EXPECT_EQ(argument.get_identifier(), kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>(3));
}
}
