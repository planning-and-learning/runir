#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/datas.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::kr::dl
{

namespace grammar = runir::kr::dl::grammar;

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

void bind_grammar_datas(nb::module_& m)
{
    bind_data<grammar::Concept<BotTag>>(m, "ConceptBotData");
    bind_data<grammar::Concept<TopTag>>(m, "ConceptTopData");
    bind_data<grammar::Concept<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<grammar::Concept<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<grammar::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<grammar::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<grammar::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<grammar::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<grammar::Concept<IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<grammar::Concept<UnionTag>>(m, "ConceptUnionData");
    bind_data<grammar::Concept<NegationTag>>(m, "ConceptNegationData");
    bind_data<grammar::Concept<ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<grammar::Concept<ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<grammar::Concept<RoleValueMapContainmentTag>>(m, "ConceptRoleValueMapContainmentData");
    bind_data<grammar::Concept<RoleValueMapEqualityTag>>(m, "ConceptRoleValueMapEqualityData");
    bind_data<grammar::Concept<NominalTag>>(m, "ConceptNominalData");
    bind_data<grammar::Role<UniversalTag>>(m, "RoleUniversalData");
    bind_data<grammar::Role<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<grammar::Role<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<grammar::Role<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<grammar::Role<AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<grammar::Role<AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<grammar::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<grammar::Role<IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<grammar::Role<UnionTag>>(m, "RoleUnionData");
    bind_data<grammar::Role<ComplementTag>>(m, "RoleComplementData");
    bind_data<grammar::Role<InverseTag>>(m, "RoleInverseData");
    bind_data<grammar::Role<CompositionTag>>(m, "RoleCompositionData");
    bind_data<grammar::Role<TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<grammar::Role<ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<grammar::Role<RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<grammar::Role<IdentityTag>>(m, "RoleIdentityData");
    bind_data<grammar::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<grammar::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<grammar::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<grammar::Boolean<NonemptyTag>>(m, "BooleanNonemptyData");
    bind_data<grammar::Numerical<CountTag>>(m, "NumericalCountData");
    bind_data<grammar::Numerical<DistanceTag>>(m, "NumericalDistanceData");
    bind_data<grammar::Constructor<ConceptTag>>(m, "ConceptData");
    bind_data<grammar::Constructor<RoleTag>>(m, "RoleData");
    bind_data<grammar::Constructor<BooleanTag>>(m, "BooleanData");
    bind_data<grammar::Constructor<NumericalTag>>(m, "NumericalData");
}

}  // namespace runir::kr::dl
