#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/uns/datas.hpp>
#include <runir/kr/dl/uns/declarations.hpp>
#include <runir/kr/dl/semantics/datas.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::uns
{

namespace
{

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = ygg::Data<T>;
    auto cls = nb::class_<Data>(m, name.c_str()).def(nb::init<>());
    if constexpr (requires { &Data::index; }) cls.def_rw("index", &Data::index);
    if constexpr (requires { &Data::identifier; }) cls.def_rw("identifier", &Data::identifier);
    if constexpr (requires { &Data::arg; }) cls.def_rw("arg", &Data::arg);
    if constexpr (requires { &Data::lhs; }) cls.def_rw("lhs", &Data::lhs);
    if constexpr (requires { &Data::mid; }) cls.def_rw("mid", &Data::mid);
    if constexpr (requires { &Data::rhs; }) cls.def_rw("rhs", &Data::rhs);
    if constexpr (requires { &Data::predicate; }) cls.def_rw("predicate", &Data::predicate);
    if constexpr (requires { &Data::polarity; }) cls.def_rw("polarity", &Data::polarity);
    if constexpr (requires { &Data::object; }) cls.def_rw("object", &Data::object);
    if constexpr (requires { &Data::objects; }) cls.def_rw("objects", &Data::objects);
    if constexpr (requires { &Data::n; }) cls.def_rw("n", &Data::n);
    if constexpr (requires { &Data::role; }) cls.def_rw("role", &Data::role);
    if constexpr (requires { &Data::concept_; }) cls.def_rw("concept", &Data::concept_);
}

}  // namespace

void bind_semantics_datas(nb::module_& m)
{
    bind_data<Concept<runir::kr::UnsFamilyTag, BotTag>>(m, "ConceptBotData");
    bind_data<Concept<runir::kr::UnsFamilyTag, TopTag>>(m, "ConceptTopData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<Concept<runir::kr::UnsFamilyTag, IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, UnionTag>>(m, "ConceptUnionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, NegationTag>>(m, "ConceptNegationData");
    bind_data<Concept<runir::kr::UnsFamilyTag, ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AtLeastNumberRestrictionTag>>(m, "ConceptAtLeastNumberRestrictionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AtMostNumberRestrictionTag>>(m, "ConceptAtMostNumberRestrictionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, ExactNumberRestrictionTag>>(m, "ConceptExactNumberRestrictionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, QualifiedAtLeastNumberRestrictionTag>>(m, "ConceptQualifiedAtLeastNumberRestrictionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, QualifiedAtMostNumberRestrictionTag>>(m, "ConceptQualifiedAtMostNumberRestrictionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, QualifiedExactNumberRestrictionTag>>(m, "ConceptQualifiedExactNumberRestrictionData");
    bind_data<Concept<runir::kr::UnsFamilyTag, RoleValueMapTag>>(m, "ConceptRoleValueMapData");
    bind_data<Concept<runir::kr::UnsFamilyTag, AgreementTag>>(m, "ConceptAgreementData");
    bind_data<Concept<runir::kr::UnsFamilyTag, RoleFillersTag>>(m, "ConceptRoleFillersData");
    bind_data<Concept<runir::kr::UnsFamilyTag, OneOfTag>>(m, "ConceptOneOfData");
    bind_data<Concept<runir::kr::UnsFamilyTag, NominalTag>>(m, "ConceptNominalData");

    bind_data<Role<runir::kr::UnsFamilyTag, UniversalTag>>(m, "RoleUniversalData");
    bind_data<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<Role<runir::kr::UnsFamilyTag, IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<Role<runir::kr::UnsFamilyTag, UnionTag>>(m, "RoleUnionData");
    bind_data<Role<runir::kr::UnsFamilyTag, ComplementTag>>(m, "RoleComplementData");
    bind_data<Role<runir::kr::UnsFamilyTag, InverseTag>>(m, "RoleInverseData");
    bind_data<Role<runir::kr::UnsFamilyTag, CompositionTag>>(m, "RoleCompositionData");
    bind_data<Role<runir::kr::UnsFamilyTag, TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<Role<runir::kr::UnsFamilyTag, ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<Role<runir::kr::UnsFamilyTag, RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<Role<runir::kr::UnsFamilyTag, IdentityTag>>(m, "RoleIdentityData");

    bind_data<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicGoalStaticData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicGoalFluentData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicGoalDerivedData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, NonemptyTag>>(m, "BooleanNonemptyData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, EqTag<BooleanTag>>>(m, "BooleanEqData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, NeqTag<BooleanTag>>>(m, "BooleanNeqData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, LtTag<BooleanTag>>>(m, "BooleanLtData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, LeTag<BooleanTag>>>(m, "BooleanLeData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, GtTag<BooleanTag>>>(m, "BooleanGtData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, GeTag<BooleanTag>>>(m, "BooleanGeData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, EqTag<NumericalTag>>>(m, "NumericalEqData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, NeqTag<NumericalTag>>>(m, "NumericalNeqData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, LtTag<NumericalTag>>>(m, "NumericalLtData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, LeTag<NumericalTag>>>(m, "NumericalLeData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, GtTag<NumericalTag>>>(m, "NumericalGtData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, GeTag<NumericalTag>>>(m, "NumericalGeData");
    bind_data<Boolean<runir::kr::UnsFamilyTag, BooleanConstantTag>>(m, "BooleanConstantData");

    bind_data<Numerical<runir::kr::UnsFamilyTag, CountTag>>(m, "NumericalCountData");
    bind_data<Numerical<runir::kr::UnsFamilyTag, DistanceTag>>(m, "NumericalDistanceData");
    bind_data<Numerical<runir::kr::UnsFamilyTag, NumericalConstantTag>>(m, "NumericalConstantData");

    bind_data<Constructor<runir::kr::UnsFamilyTag, ConceptTag>>(m, "ConceptData");
    bind_data<Constructor<runir::kr::UnsFamilyTag, RoleTag>>(m, "RoleData");
    bind_data<Constructor<runir::kr::UnsFamilyTag, BooleanTag>>(m, "BooleanData");
    bind_data<Constructor<runir::kr::UnsFamilyTag, NumericalTag>>(m, "NumericalData");
}

}  // namespace runir::kr::dl::uns
