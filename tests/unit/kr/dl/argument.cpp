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

using ArgumentTypes = ygg::TypeList<kr::dl::Argument<kr::dl::ConceptTag>,
                                    kr::dl::Argument<kr::dl::RoleTag>,
                                    kr::dl::Argument<kr::dl::BooleanTag>,
                                    kr::dl::Argument<kr::dl::NumericalTag>>;
using ArgumentIdentifierTypes = ygg::TypeList<kr::dl::ArgumentIdentifier<kr::dl::ConceptTag>,
                                              kr::dl::ArgumentIdentifier<kr::dl::RoleTag>,
                                              kr::dl::ArgumentIdentifier<kr::dl::BooleanTag>,
                                              kr::dl::ArgumentIdentifier<kr::dl::NumericalTag>>;

template<typename... Ts>
consteval bool identifiers(ygg::TypeList<Ts...>)
{
    return ((std::constructible_from<Ts, ygg::uint_t> && ygg::Identifiable<Ts> && std::totally_ordered<Ts>) && ...);
}

template<typename... Categories>
consteval bool view_aliases(ygg::TypeList<Categories...>)
{
    return (std::same_as<kr::dl::ArgumentView<Categories>, ygg::View<ygg::Index<kr::dl::Argument<Categories>>, kr::dl::ExtConstructorRepository>> && ...);
}

static_assert(indexed_data_views<kr::dl::ExtConstructorRepository>(ArgumentTypes {}));
static_assert(identifiers(ArgumentIdentifierTypes {}));
static_assert(view_aliases(kr::dl::CategoryTags {}));

}  // namespace

TEST(RunirKrDlArgument, ExposesNameAndIdentifier)
{
    namespace dl = kr::dl;
    auto planning_repository = tyr::formalism::planning::RepositoryFactory().create_shared();
    auto repository = dl::ExtConstructorRepositoryFactory().create(planning_repository);
    auto data = ygg::Data<dl::Argument<dl::ConceptTag>>(std::string("source"), dl::ArgumentIdentifier<dl::ConceptTag>(3));
    const auto argument = repository->get_or_create(data).first;

    EXPECT_EQ(argument.get_name(), "source");
    EXPECT_EQ(argument.get_identifier(), dl::ArgumentIdentifier<dl::ConceptTag>(3));
}

}
