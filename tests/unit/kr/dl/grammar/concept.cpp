#include "runir/kr/dl/grammar/declarations.hpp"

#include <concepts>
#include <runir/kr/dl/grammar/concept_data.hpp>
#include <runir/kr/dl/grammar/concept_view.hpp>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <tyr/formalism/planning/repository.hpp>
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
    && urd::Identifiable<urd::view<T, Repository>> && std::totally_ordered<urd::index<T>> && std::totally_ordered<urd::data<T>>
    && std::totally_ordered<urd::view<T, Repository>> && requires(urd::data<T>& data, const urd::view<T, Repository>& view) {
           data.index;
           view.index();
       };

template<typename... Ts>
consteval bool indexed_data_views(boost::mp11::mp_list<Ts...>)
{ return (IndexedDataView<Ts, kr::dl::grammar::ConstructorRepository> && ...); }

static_assert(indexed_data_views<>(kr::dl::grammar::ConceptTypes {}));

using NullaryTags = boost::mp11::mp_list<kr::dl::BotTag, kr::dl::TopTag>;
using PredicateTags = boost::mp11::mp_list<kr::dl::AtomicStateTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicStateTag<tyr::formalism::DerivedTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::StaticTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::FluentTag>,
                                    kr::dl::AtomicGoalTag<tyr::formalism::DerivedTag>>;
using BinaryTags = boost::mp11::mp_list<kr::dl::IntersectionTag,
                                 kr::dl::UnionTag,
                                 kr::dl::ValueRestrictionTag,
                                 kr::dl::ExistentialQuantificationTag,
                                 kr::dl::RoleValueMapTag,
                                 kr::dl::AgreementTag>;
using NumberRestrictionTags = boost::mp11::mp_list<kr::dl::AtLeastNumberRestrictionTag, kr::dl::AtMostNumberRestrictionTag, kr::dl::ExactNumberRestrictionTag>;
using QualifiedNumberRestrictionTags =
    boost::mp11::mp_list<kr::dl::QualifiedAtLeastNumberRestrictionTag, kr::dl::QualifiedAtMostNumberRestrictionTag, kr::dl::QualifiedExactNumberRestrictionTag>;

template<typename Tag>
consteval bool concept_data_view()
{
    using Entity = kr::dl::grammar::Concept<Tag>;
    using Data = urd::data<Entity>;
    using View = urd::view<Entity, kr::dl::grammar::ConstructorRepository>;

    static_assert(std::same_as<View, kr::dl::grammar::ConceptView<Tag>>);
    if constexpr (false)
        static_assert(std::same_as<View, kr::dl::grammar::ConceptView<Tag>>);

    if constexpr (boost::mp11::mp_contains<NullaryTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.index;
            view.index();
        };
    else if constexpr (boost::mp11::mp_contains<PredicateTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.predicate;
            data.polarity;
            view.get_predicate();
            view.get_polarity();
        };
    else if constexpr (boost::mp11::mp_contains<BinaryTags, Tag>::value)
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
    else if constexpr (boost::mp11::mp_contains<NumberRestrictionTags, Tag>::value)
        return requires(Data& data, const View& view) {
            data.n;
            data.role;
            view.get_n();
            view.get_role();
        };
    else if constexpr (boost::mp11::mp_contains<QualifiedNumberRestrictionTags, Tag>::value)
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

template<typename... Tags>
consteval bool concept_data_views(boost::mp11::mp_list<Tags...>)
{ return (concept_data_view<Tags>() && ...); }

static_assert(concept_data_views(kr::dl::GenerationConceptConstructorTags {}));

}  // namespace

}
