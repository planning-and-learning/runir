#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/datas.hpp>
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
}

}  // namespace

void bind_grammar_datas(nb::module_& m)
{
    bind_data<runir::kr::dl::grammar::Concept<BotTag>>(m, "ConceptBotData");
    bind_data<runir::kr::dl::grammar::Concept<TopTag>>(m, "ConceptTopData");
    bind_data<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<runir::kr::dl::grammar::Concept<IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<runir::kr::dl::grammar::Concept<UnionTag>>(m, "ConceptUnionData");
    bind_data<runir::kr::dl::grammar::Concept<NegationTag>>(m, "ConceptNegationData");
    bind_data<runir::kr::dl::grammar::Concept<ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<runir::kr::dl::grammar::Concept<RoleValueMapContainmentTag>>(m, "ConceptRoleValueMapContainmentData");
    bind_data<runir::kr::dl::grammar::Concept<RoleValueMapEqualityTag>>(m, "ConceptRoleValueMapEqualityData");
    bind_data<runir::kr::dl::grammar::Concept<NominalTag>>(m, "ConceptNominalData");
    bind_data<runir::kr::dl::grammar::Role<UniversalTag>>(m, "RoleUniversalData");
    bind_data<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<runir::kr::dl::grammar::Role<IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<runir::kr::dl::grammar::Role<UnionTag>>(m, "RoleUnionData");
    bind_data<runir::kr::dl::grammar::Role<ComplementTag>>(m, "RoleComplementData");
    bind_data<runir::kr::dl::grammar::Role<InverseTag>>(m, "RoleInverseData");
    bind_data<runir::kr::dl::grammar::Role<CompositionTag>>(m, "RoleCompositionData");
    bind_data<runir::kr::dl::grammar::Role<TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<runir::kr::dl::grammar::Role<ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<runir::kr::dl::grammar::Role<RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<runir::kr::dl::grammar::Role<IdentityTag>>(m, "RoleIdentityData");
    bind_data<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Boolean<NonemptyTag>>(m, "BooleanNonemptyData");
    bind_data<runir::kr::dl::grammar::Numerical<CountTag>>(m, "NumericalCountData");
    bind_data<runir::kr::dl::grammar::Numerical<DistanceTag>>(m, "NumericalDistanceData");
    bind_data<runir::kr::dl::grammar::Constructor<ConceptTag>>(m, "ConceptData");
    bind_data<runir::kr::dl::grammar::Constructor<RoleTag>>(m, "RoleData");
    bind_data<runir::kr::dl::grammar::Constructor<BooleanTag>>(m, "BooleanData");
    bind_data<runir::kr::dl::grammar::Constructor<NumericalTag>>(m, "NumericalData");
}

}  // namespace runir::kr::dl
