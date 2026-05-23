#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/semantics/datas.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::kr::dl
{

namespace
{

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = tyr::Data<T>;

    auto cls = nb::class_<Data>(m, name.c_str()).def(nb::init<>());

    if constexpr (requires { &Data::index; })
        cls.def_rw("index", &Data::index);
    if constexpr (requires { &Data::arg; })
        cls.def_rw("arg", &Data::arg);
    if constexpr (requires { &Data::lhs; })
        cls.def_rw("lhs", &Data::lhs);
    if constexpr (requires { &Data::mid; })
        cls.def_rw("mid", &Data::mid);
    if constexpr (requires { &Data::rhs; })
        cls.def_rw("rhs", &Data::rhs);
    if constexpr (requires { &Data::predicate; })
        cls.def_rw("predicate", &Data::predicate);
    if constexpr (requires { &Data::polarity; })
        cls.def_rw("polarity", &Data::polarity);
    if constexpr (requires { &Data::object; })
        cls.def_rw("object", &Data::object);
    if constexpr (requires { &Data::objects; })
        cls.def_rw("objects", &Data::objects);
    if constexpr (requires { &Data::n; })
        cls.def_rw("n", &Data::n);
    if constexpr (requires { &Data::role; })
        cls.def_rw("role", &Data::role);
    if constexpr (requires { &Data::concept_; })
        cls.def_rw("concept", &Data::concept_);
}

}  // namespace

void bind_semantics_datas(nb::module_& m)
{
    bind_data<Concept<BaseFamilyTag, BotTag>>(m, "ConceptBotData");
    bind_data<Concept<BaseFamilyTag, TopTag>>(m, "ConceptTopData");
    bind_data<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<Concept<BaseFamilyTag, IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<Concept<BaseFamilyTag, UnionTag>>(m, "ConceptUnionData");
    bind_data<Concept<BaseFamilyTag, NegationTag>>(m, "ConceptNegationData");
    bind_data<Concept<BaseFamilyTag, ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<Concept<BaseFamilyTag, ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<Concept<BaseFamilyTag, AtLeastNumberRestrictionTag>>(m, "ConceptAtLeastNumberRestrictionData");
    bind_data<Concept<BaseFamilyTag, AtMostNumberRestrictionTag>>(m, "ConceptAtMostNumberRestrictionData");
    bind_data<Concept<BaseFamilyTag, ExactNumberRestrictionTag>>(m, "ConceptExactNumberRestrictionData");
    bind_data<Concept<BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>>(m, "ConceptQualifiedAtLeastNumberRestrictionData");
    bind_data<Concept<BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>>(m, "ConceptQualifiedAtMostNumberRestrictionData");
    bind_data<Concept<BaseFamilyTag, QualifiedExactNumberRestrictionTag>>(m, "ConceptQualifiedExactNumberRestrictionData");
    bind_data<Concept<BaseFamilyTag, RoleValueMapTag>>(m, "ConceptRoleValueMapData");
    bind_data<Concept<BaseFamilyTag, AgreementTag>>(m, "ConceptAgreementData");
    bind_data<Concept<BaseFamilyTag, RoleFillersTag>>(m, "ConceptRoleFillersData");
    bind_data<Concept<BaseFamilyTag, OneOfTag>>(m, "ConceptOneOfData");
    bind_data<Concept<BaseFamilyTag, NominalTag>>(m, "ConceptNominalData");

    bind_data<Role<BaseFamilyTag, UniversalTag>>(m, "RoleUniversalData");
    bind_data<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<Role<BaseFamilyTag, IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<Role<BaseFamilyTag, UnionTag>>(m, "RoleUnionData");
    bind_data<Role<BaseFamilyTag, ComplementTag>>(m, "RoleComplementData");
    bind_data<Role<BaseFamilyTag, InverseTag>>(m, "RoleInverseData");
    bind_data<Role<BaseFamilyTag, CompositionTag>>(m, "RoleCompositionData");
    bind_data<Role<BaseFamilyTag, TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<Role<BaseFamilyTag, ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<Role<BaseFamilyTag, RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<Role<BaseFamilyTag, IdentityTag>>(m, "RoleIdentityData");

    bind_data<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicGoalStaticData");
    bind_data<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicGoalFluentData");
    bind_data<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicGoalDerivedData");
    bind_data<Boolean<BaseFamilyTag, NonemptyTag>>(m, "BooleanNonemptyData");

    bind_data<Numerical<BaseFamilyTag, CountTag>>(m, "NumericalCountData");
    bind_data<Numerical<BaseFamilyTag, DistanceTag>>(m, "NumericalDistanceData");

    bind_data<Constructor<BaseFamilyTag, ConceptTag>>(m, "ConceptData");
    bind_data<Constructor<BaseFamilyTag, RoleTag>>(m, "RoleData");
    bind_data<Constructor<BaseFamilyTag, BooleanTag>>(m, "BooleanData");
    bind_data<Constructor<BaseFamilyTag, NumericalTag>>(m, "NumericalData");
}

}  // namespace runir::kr::dl
