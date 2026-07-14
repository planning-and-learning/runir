#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/datas.hpp>
#include <runir/kr/dl/declarations.hpp>
#include <runir/kr/dl/semantics/datas.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::ext
{

namespace
{

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = ygg::Data<T>;

    auto cls = nb::class_<Data>(m, name.c_str()).def(nb::init<>());

    if constexpr (requires { &Data::index; })
        cls.def_rw("index", &Data::index);
    if constexpr (requires { &Data::reference; })
        cls.def_rw("reference", &Data::reference);
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
    bind_data<Concept<runir::kr::ExtFamilyTag, BotTag>>(m, "ConceptBotData");
    bind_data<Concept<runir::kr::ExtFamilyTag, TopTag>>(m, "ConceptTopData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<Concept<runir::kr::ExtFamilyTag, IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, UnionTag>>(m, "ConceptUnionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, NegationTag>>(m, "ConceptNegationData");
    bind_data<Concept<runir::kr::ExtFamilyTag, ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AtLeastNumberRestrictionTag>>(m, "ConceptAtLeastNumberRestrictionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AtMostNumberRestrictionTag>>(m, "ConceptAtMostNumberRestrictionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, ExactNumberRestrictionTag>>(m, "ConceptExactNumberRestrictionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, QualifiedAtLeastNumberRestrictionTag>>(m, "ConceptQualifiedAtLeastNumberRestrictionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, QualifiedAtMostNumberRestrictionTag>>(m, "ConceptQualifiedAtMostNumberRestrictionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, QualifiedExactNumberRestrictionTag>>(m, "ConceptQualifiedExactNumberRestrictionData");
    bind_data<Concept<runir::kr::ExtFamilyTag, RoleValueMapTag>>(m, "ConceptRoleValueMapData");
    bind_data<Concept<runir::kr::ExtFamilyTag, AgreementTag>>(m, "ConceptAgreementData");
    bind_data<Concept<runir::kr::ExtFamilyTag, RoleFillersTag>>(m, "ConceptRoleFillersData");
    bind_data<Concept<runir::kr::ExtFamilyTag, OneOfTag>>(m, "ConceptOneOfData");
    bind_data<Concept<runir::kr::ExtFamilyTag, NominalTag>>(m, "ConceptNominalData");
    bind_data<Concept<runir::kr::ExtFamilyTag, RegisterTag>>(m, "ConceptRegisterData");
    bind_data<Concept<runir::kr::ExtFamilyTag, ArgumentTag<ConceptTag>>>(m, "ConceptArgumentData");

    bind_data<Role<runir::kr::ExtFamilyTag, UniversalTag>>(m, "RoleUniversalData");
    bind_data<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<Role<runir::kr::ExtFamilyTag, IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<Role<runir::kr::ExtFamilyTag, UnionTag>>(m, "RoleUnionData");
    bind_data<Role<runir::kr::ExtFamilyTag, ComplementTag>>(m, "RoleComplementData");
    bind_data<Role<runir::kr::ExtFamilyTag, InverseTag>>(m, "RoleInverseData");
    bind_data<Role<runir::kr::ExtFamilyTag, CompositionTag>>(m, "RoleCompositionData");
    bind_data<Role<runir::kr::ExtFamilyTag, TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<Role<runir::kr::ExtFamilyTag, ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<Role<runir::kr::ExtFamilyTag, RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<Role<runir::kr::ExtFamilyTag, IdentityTag>>(m, "RoleIdentityData");
    bind_data<Role<runir::kr::ExtFamilyTag, RegisterTag>>(m, "RoleRegisterData");
    bind_data<Role<runir::kr::ExtFamilyTag, ArgumentTag<RoleTag>>>(m, "RoleArgumentData");

    bind_data<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicGoalStaticData");
    bind_data<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicGoalFluentData");
    bind_data<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicGoalDerivedData");
    bind_data<Boolean<runir::kr::ExtFamilyTag, NonemptyTag>>(m, "BooleanNonemptyData");
    bind_data<Boolean<runir::kr::ExtFamilyTag, ArgumentTag<BooleanTag>>>(m, "BooleanArgumentData");

    bind_data<Numerical<runir::kr::ExtFamilyTag, CountTag>>(m, "NumericalCountData");
    bind_data<Numerical<runir::kr::ExtFamilyTag, DistanceTag>>(m, "NumericalDistanceData");
    bind_data<Numerical<runir::kr::ExtFamilyTag, ArgumentTag<NumericalTag>>>(m, "NumericalArgumentData");

    bind_data<Constructor<runir::kr::ExtFamilyTag, ConceptTag>>(m, "ConceptData");
    bind_data<Constructor<runir::kr::ExtFamilyTag, RoleTag>>(m, "RoleData");
    bind_data<Constructor<runir::kr::ExtFamilyTag, BooleanTag>>(m, "BooleanData");
    bind_data<Constructor<runir::kr::ExtFamilyTag, NumericalTag>>(m, "NumericalData");
}

}  // namespace runir::kr::dl::ext
