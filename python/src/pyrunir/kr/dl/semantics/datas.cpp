#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/knowledge_representation/dl/semantics/datas.hpp>

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
}

}  // namespace

void bind_semantics_datas(nb::module_& m)
{
    bind_data<Concept<BotTag>>(m, "ConceptBotData");
    bind_data<Concept<TopTag>>(m, "ConceptTopData");
    bind_data<Concept<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<Concept<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<Concept<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<Concept<AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<Concept<AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<Concept<IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<Concept<UnionTag>>(m, "ConceptUnionData");
    bind_data<Concept<NegationTag>>(m, "ConceptNegationData");
    bind_data<Concept<ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<Concept<ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<Concept<RoleValueMapContainmentTag>>(m, "ConceptRoleValueMapContainmentData");
    bind_data<Concept<RoleValueMapEqualityTag>>(m, "ConceptRoleValueMapEqualityData");
    bind_data<Concept<NominalTag>>(m, "ConceptNominalData");

    bind_data<Role<UniversalTag>>(m, "RoleUniversalData");
    bind_data<Role<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<Role<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<Role<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<Role<AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<Role<AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<Role<AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<Role<IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<Role<UnionTag>>(m, "RoleUnionData");
    bind_data<Role<ComplementTag>>(m, "RoleComplementData");
    bind_data<Role<InverseTag>>(m, "RoleInverseData");
    bind_data<Role<CompositionTag>>(m, "RoleCompositionData");
    bind_data<Role<TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<Role<ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<Role<RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<Role<IdentityTag>>(m, "RoleIdentityData");

    bind_data<Boolean<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<Boolean<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<Boolean<NonemptyTag>>(m, "BooleanNonemptyData");

    bind_data<Numerical<CountTag>>(m, "NumericalCountData");
    bind_data<Numerical<DistanceTag>>(m, "NumericalDistanceData");

    bind_data<Constructor<ConceptTag>>(m, "ConceptData");
    bind_data<Constructor<RoleTag>>(m, "RoleData");
    bind_data<Constructor<BooleanTag>>(m, "BooleanData");
    bind_data<Constructor<NumericalTag>>(m, "NumericalData");
}

}  // namespace runir::kr::dl
