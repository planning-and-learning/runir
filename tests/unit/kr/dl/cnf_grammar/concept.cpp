#include <concepts>
#include <runir/kr/dl/cnf_grammar/concept_data.hpp>
#include <runir/kr/dl/cnf_grammar/concept_index.hpp>
#include <runir/kr/dl/cnf_grammar/concept_view.hpp>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
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
           data.clear();
           view.get_index();
       };

template<typename Family, typename... Ts>
consteval bool indexed_data_views(ygg::TypeList<Ts...>)
{
    return (IndexedDataView<Ts, kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>> && ...);
}

static_assert(indexed_data_views<kr::BaseFamilyTag>(kr::dl::cnf_grammar::FamilyConceptTypes<kr::BaseFamilyTag> {}));
static_assert(indexed_data_views<kr::ExtFamilyTag>(kr::dl::cnf_grammar::FamilyConceptTypes<kr::ExtFamilyTag> {}));
static_assert(indexed_data_views<kr::UnsFamilyTag>(kr::dl::cnf_grammar::FamilyConceptTypes<kr::UnsFamilyTag> {}));

using NullaryTags = ygg::TypeList<kr::dl::BotTag, kr::dl::TopTag>;
using PredicateTags = ygg::TypeList<kr::dl::AtomicStateTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>;
using BinaryTags = ygg::TypeList<kr::dl::IntersectionTag,
                                 kr::dl::UnionTag,
                                 kr::dl::ValueRestrictionTag,
                                 kr::dl::ExistentialQuantificationTag,
                                 kr::dl::RoleValueMapTag,
                                 kr::dl::AgreementTag>;
using NumberRestrictionTags = ygg::TypeList<kr::dl::AtLeastNumberRestrictionTag, kr::dl::AtMostNumberRestrictionTag, kr::dl::ExactNumberRestrictionTag>;
using QualifiedNumberRestrictionTags =
    ygg::TypeList<kr::dl::QualifiedAtLeastNumberRestrictionTag, kr::dl::QualifiedAtMostNumberRestrictionTag, kr::dl::QualifiedExactNumberRestrictionTag>;

template<typename Family, typename Tag>
consteval bool concept_data_view()
{
    using Entity = kr::dl::cnf_grammar::Concept<Family, Tag>;
    using Data = ygg::Data<Entity>;
    using View = ygg::View<ygg::Index<Entity>, kr::dl::cnf_grammar::ConstructorRepositoryFor<Family>>;

    static_assert(std::same_as<View, kr::dl::cnf_grammar::FamilyConceptView<Family, Tag>>);
    if constexpr (std::same_as<Family, kr::BaseFamilyTag>)
        static_assert(std::same_as<View, kr::dl::cnf_grammar::BaseConceptView<Tag>>);

    if constexpr (kr::dl::TypeListContains<Tag, NullaryTags>::value)
        return requires(Data& data, const View& view) {
            data.index;
            view.get_index();
        };
    else if constexpr (kr::dl::TypeListContains<Tag, PredicateTags>::value)
        return requires(Data& data, const View& view) {
            data.predicate;
            data.polarity;
            view.get_predicate();
            view.get_polarity();
        };
    else if constexpr (kr::dl::TypeListContains<Tag, BinaryTags>::value)
        return requires(Data& data, const View& view) {
            data.lhs;
            data.rhs;
            view.get_lhs();
            view.get_rhs();
        };
    else if constexpr (std::same_as<Tag, kr::dl::NegationTag>)
        return requires(Data& data, const View& view) {
            data.arg;
            view.get_arg();
        };
    else if constexpr (kr::dl::TypeListContains<Tag, NumberRestrictionTags>::value)
        return requires(Data& data, const View& view) {
            data.n;
            data.role;
            view.get_n();
            view.get_role();
        };
    else if constexpr (kr::dl::TypeListContains<Tag, QualifiedNumberRestrictionTags>::value)
        return requires(Data& data, const View& view) {
            data.n;
            data.role;
            data.concept_;
            view.get_n();
            view.get_role();
            view.get_concept();
        };
    else if constexpr (std::same_as<Tag, kr::dl::RoleFillersTag>)
        return requires(Data& data, const View& view) {
            data.role;
            data.objects;
            view.get_role();
            view.get_objects();
        };
    else if constexpr (std::same_as<Tag, kr::dl::OneOfTag>)
        return requires(Data& data, const View& view) {
            data.objects;
            view.get_objects();
        };
    else if constexpr (std::same_as<Tag, kr::dl::NominalTag>)
        return requires(Data& data, const View& view) {
            data.object;
            view.get_object();
        };
    else if constexpr (std::same_as<Tag, kr::dl::RegisterTag> || std::same_as<Tag, kr::dl::ArgumentTag<kr::dl::ConceptTag>>)
        return requires(Data& data, const View& view) {
            data.identifier;
            view.get_identifier();
        };
    else
        return false;
}

template<typename Family, typename... Tags>
consteval bool concept_data_views(ygg::TypeList<Tags...>)
{
    return (concept_data_view<Family, Tags>() && ...);
}

static_assert(concept_data_views<kr::BaseFamilyTag>(kr::dl::FamilyConceptConstructorTags<kr::BaseFamilyTag> {}));
static_assert(concept_data_views<kr::ExtFamilyTag>(kr::dl::FamilyConceptConstructorTags<kr::ExtFamilyTag> {}));
static_assert(concept_data_views<kr::UnsFamilyTag>(kr::dl::FamilyConceptConstructorTags<kr::UnsFamilyTag> {}));

}  // namespace

}
