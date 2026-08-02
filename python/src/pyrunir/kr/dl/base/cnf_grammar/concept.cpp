#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/cnf_grammar/concept_view.hpp>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{
namespace
{

template<typename Tag>
void bind_concept_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def_rw("predicate", &Data::predicate).def_rw("polarity", &Data::polarity);
    else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag> || std::same_as<Tag, ValueRestrictionTag>
                       || std::same_as<Tag, ExistentialQuantificationTag> || std::same_as<Tag, RoleValueMapTag> || std::same_as<Tag, AgreementTag>)
        cls.def_rw("lhs", &Data::lhs).def_rw("rhs", &Data::rhs);
    else if constexpr (std::same_as<Tag, NegationTag>)
        cls.def_rw("arg", &Data::arg);
    else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                       || std::same_as<Tag, ExactNumberRestrictionTag>)
        cls.def_rw("n", &Data::n).def_rw("role", &Data::role);
    else if constexpr (std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>
                       || std::same_as<Tag, QualifiedExactNumberRestrictionTag>)
        cls.def_rw("n", &Data::n).def_rw("role", &Data::role).def_rw("concept", &Data::concept_);
    else if constexpr (std::same_as<Tag, RoleFillersTag>)
        cls.def_rw("role", &Data::role).def_rw("objects", &Data::objects);
    else if constexpr (std::same_as<Tag, OneOfTag>)
        cls.def_rw("objects", &Data::objects);
    else if constexpr (std::same_as<Tag, NominalTag>)
        cls.def_rw("object", &Data::object);
}

template<typename Tag>
void bind_concept_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, Tag>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>()).def("get_polarity", &View::get_polarity);
    else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag> || std::same_as<Tag, ValueRestrictionTag>
                       || std::same_as<Tag, ExistentialQuantificationTag> || std::same_as<Tag, RoleValueMapTag> || std::same_as<Tag, AgreementTag>)
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>()).def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, NegationTag>)
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, AtLeastNumberRestrictionTag> || std::same_as<Tag, AtMostNumberRestrictionTag>
                       || std::same_as<Tag, ExactNumberRestrictionTag>)
        cls.def("get_n", &View::get_n).def("get_role", &View::get_role, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, QualifiedAtMostNumberRestrictionTag>
                       || std::same_as<Tag, QualifiedExactNumberRestrictionTag>)
        cls.def("get_n", &View::get_n).def("get_role", &View::get_role, nb::keep_alive<0, 1>()).def("get_concept", &View::get_concept, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, RoleFillersTag>)
        cls.def("get_role", &View::get_role, nb::keep_alive<0, 1>()).def("get_objects", &View::get_objects);
    else if constexpr (std::same_as<Tag, OneOfTag>)
        cls.def("get_objects", &View::get_objects);
    else if constexpr (std::same_as<Tag, NominalTag>)
        cls.def("get_object", &View::get_object, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_cnf_grammar_concept(nb::module_& m)
{
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, BotTag>>>(m, "ConceptBotIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, TopTag>>>(m, "ConceptTopIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>>(
        m,
        "ConceptAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>>(
        m,
        "ConceptAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>>(
        m,
        "ConceptAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>>(
        m,
        "ConceptAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>>(
        m,
        "ConceptAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>>(
        m,
        "ConceptAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, IntersectionTag>>>(m, "ConceptIntersectionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, UnionTag>>>(m, "ConceptUnionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, NegationTag>>>(m, "ConceptNegationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ValueRestrictionTag>>>(m, "ConceptValueRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ExistentialQuantificationTag>>>(
        m,
        "ConceptExistentialQuantificationIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtLeastNumberRestrictionTag>>>(
        m,
        "ConceptAtLeastNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtMostNumberRestrictionTag>>>(
        m,
        "ConceptAtMostNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ExactNumberRestrictionTag>>>(m,
                                                                                                                          "ConceptExactNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>>>(
        m,
        "ConceptQualifiedAtLeastNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>>>(
        m,
        "ConceptQualifiedAtMostNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedExactNumberRestrictionTag>>>(
        m,
        "ConceptQualifiedExactNumberRestrictionIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, RoleValueMapTag>>>(m, "ConceptRoleValueMapIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AgreementTag>>>(m, "ConceptAgreementIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, RoleFillersTag>>>(m, "ConceptRoleFillersIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, OneOfTag>>>(m, "ConceptOneOfIndex");
    ygg::bind_index<ygg::Index<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, NominalTag>>>(m, "ConceptNominalIndex");

    bind_concept_data<BotTag>(m, "ConceptBotData");
    bind_concept_data<TopTag>(m, "ConceptTopData");
    bind_concept_data<AtomicStateTag<tyr::formalism::StaticTag>>(m, "ConceptAtomicStateStaticData");
    bind_concept_data<AtomicStateTag<tyr::formalism::FluentTag>>(m, "ConceptAtomicStateFluentData");
    bind_concept_data<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "ConceptAtomicStateDerivedData");
    bind_concept_data<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "ConceptAtomicGoalStaticData");
    bind_concept_data<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "ConceptAtomicGoalFluentData");
    bind_concept_data<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "ConceptAtomicGoalDerivedData");
    bind_concept_data<IntersectionTag>(m, "ConceptIntersectionData");
    bind_concept_data<UnionTag>(m, "ConceptUnionData");
    bind_concept_data<NegationTag>(m, "ConceptNegationData");
    bind_concept_data<ValueRestrictionTag>(m, "ConceptValueRestrictionData");
    bind_concept_data<ExistentialQuantificationTag>(m, "ConceptExistentialQuantificationData");
    bind_concept_data<AtLeastNumberRestrictionTag>(m, "ConceptAtLeastNumberRestrictionData");
    bind_concept_data<AtMostNumberRestrictionTag>(m, "ConceptAtMostNumberRestrictionData");
    bind_concept_data<ExactNumberRestrictionTag>(m, "ConceptExactNumberRestrictionData");
    bind_concept_data<QualifiedAtLeastNumberRestrictionTag>(m, "ConceptQualifiedAtLeastNumberRestrictionData");
    bind_concept_data<QualifiedAtMostNumberRestrictionTag>(m, "ConceptQualifiedAtMostNumberRestrictionData");
    bind_concept_data<QualifiedExactNumberRestrictionTag>(m, "ConceptQualifiedExactNumberRestrictionData");
    bind_concept_data<RoleValueMapTag>(m, "ConceptRoleValueMapData");
    bind_concept_data<AgreementTag>(m, "ConceptAgreementData");
    bind_concept_data<RoleFillersTag>(m, "ConceptRoleFillersData");
    bind_concept_data<OneOfTag>(m, "ConceptOneOfData");
    bind_concept_data<NominalTag>(m, "ConceptNominalData");

    bind_concept_view<BotTag>(m, "ConceptBot");
    bind_concept_view<TopTag>(m, "ConceptTop");
    bind_concept_view<AtomicStateTag<tyr::formalism::StaticTag>>(m, "ConceptAtomicStateStatic");
    bind_concept_view<AtomicStateTag<tyr::formalism::FluentTag>>(m, "ConceptAtomicStateFluent");
    bind_concept_view<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "ConceptAtomicStateDerived");
    bind_concept_view<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "ConceptAtomicGoalStatic");
    bind_concept_view<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "ConceptAtomicGoalFluent");
    bind_concept_view<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "ConceptAtomicGoalDerived");
    bind_concept_view<IntersectionTag>(m, "ConceptIntersection");
    bind_concept_view<UnionTag>(m, "ConceptUnion");
    bind_concept_view<NegationTag>(m, "ConceptNegation");
    bind_concept_view<ValueRestrictionTag>(m, "ConceptValueRestriction");
    bind_concept_view<ExistentialQuantificationTag>(m, "ConceptExistentialQuantification");
    bind_concept_view<AtLeastNumberRestrictionTag>(m, "ConceptAtLeastNumberRestriction");
    bind_concept_view<AtMostNumberRestrictionTag>(m, "ConceptAtMostNumberRestriction");
    bind_concept_view<ExactNumberRestrictionTag>(m, "ConceptExactNumberRestriction");
    bind_concept_view<QualifiedAtLeastNumberRestrictionTag>(m, "ConceptQualifiedAtLeastNumberRestriction");
    bind_concept_view<QualifiedAtMostNumberRestrictionTag>(m, "ConceptQualifiedAtMostNumberRestriction");
    bind_concept_view<QualifiedExactNumberRestrictionTag>(m, "ConceptQualifiedExactNumberRestriction");
    bind_concept_view<RoleValueMapTag>(m, "ConceptRoleValueMap");
    bind_concept_view<AgreementTag>(m, "ConceptAgreement");
    bind_concept_view<RoleFillersTag>(m, "ConceptRoleFillers");
    bind_concept_view<OneOfTag>(m, "ConceptOneOf");
    bind_concept_view<NominalTag>(m, "ConceptNominal");
}

}  // namespace runir::kr::dl::base
