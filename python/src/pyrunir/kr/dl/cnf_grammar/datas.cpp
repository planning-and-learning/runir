#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/datas.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::kr::dl
{

namespace cnf = runir::kr::dl::cnf_grammar;

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

void bind_cnf_grammar_datas(nb::module_& m)
{
    bind_data<cnf::Concept<BotTag>>(m, "ConceptBotData");
    bind_data<cnf::Concept<TopTag>>(m, "ConceptTopData");
    bind_data<cnf::Concept<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<cnf::Concept<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<cnf::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<cnf::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<cnf::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<cnf::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<cnf::Concept<IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<cnf::Concept<UnionTag>>(m, "ConceptUnionData");
    bind_data<cnf::Concept<NegationTag>>(m, "ConceptNegationData");
    bind_data<cnf::Concept<ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<cnf::Concept<ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<cnf::Concept<RoleValueMapContainmentTag>>(m, "ConceptRoleValueMapContainmentData");
    bind_data<cnf::Concept<RoleValueMapEqualityTag>>(m, "ConceptRoleValueMapEqualityData");
    bind_data<cnf::Concept<NominalTag>>(m, "ConceptNominalData");
    bind_data<cnf::Role<UniversalTag>>(m, "RoleUniversalData");
    bind_data<cnf::Role<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<cnf::Role<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<cnf::Role<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<cnf::Role<AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<cnf::Role<AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<cnf::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<cnf::Role<IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<cnf::Role<UnionTag>>(m, "RoleUnionData");
    bind_data<cnf::Role<ComplementTag>>(m, "RoleComplementData");
    bind_data<cnf::Role<InverseTag>>(m, "RoleInverseData");
    bind_data<cnf::Role<CompositionTag>>(m, "RoleCompositionData");
    bind_data<cnf::Role<TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<cnf::Role<ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<cnf::Role<RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<cnf::Role<IdentityTag>>(m, "RoleIdentityData");
    bind_data<cnf::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<cnf::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<cnf::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<cnf::Boolean<NonemptyTag>>(m, "BooleanNonemptyData");
    bind_data<cnf::Numerical<CountTag>>(m, "NumericalCountData");
    bind_data<cnf::Numerical<DistanceTag>>(m, "NumericalDistanceData");
    bind_data<cnf::Constructor<ConceptTag>>(m, "ConceptData");
    bind_data<cnf::Constructor<RoleTag>>(m, "RoleData");
    bind_data<cnf::Constructor<BooleanTag>>(m, "BooleanData");
    bind_data<cnf::Constructor<NumericalTag>>(m, "NumericalData");
}

}  // namespace runir::kr::dl
