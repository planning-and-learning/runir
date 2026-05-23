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
    if constexpr (requires { &Data::objects; })
        cls.def_rw("objects", &Data::objects);
    if constexpr (requires { &Data::n; })
        cls.def_rw("n", &Data::n);
    if constexpr (requires { &Data::role; })
        cls.def_rw("role", &Data::role);
    if constexpr (requires { &Data::concept_; })
        cls.def_rw("concept", &Data::concept_);
    if constexpr (requires { &Data::name; })
        cls.def_rw("name", &Data::name);
    if constexpr (requires { &Data::value; })
        cls.def_rw("value", &Data::value);
}

}  // namespace

void bind_grammar_datas(nb::module_& m)
{
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, BotTag>>(m, "ConceptBotData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, TopTag>>(m, "ConceptTopData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, UnionTag>>(m, "ConceptUnionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, NegationTag>>(m, "ConceptNegationData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtLeastNumberRestrictionTag>>(m, "ConceptAtLeastNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AtMostNumberRestrictionTag>>(m, "ConceptAtMostNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, ExactNumberRestrictionTag>>(m, "ConceptExactNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>>(m, "ConceptQualifiedAtLeastNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>>(m, "ConceptQualifiedAtMostNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, QualifiedExactNumberRestrictionTag>>(m, "ConceptQualifiedExactNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, RoleValueMapTag>>(m, "ConceptRoleValueMapData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, AgreementTag>>(m, "ConceptAgreementData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, RoleFillersTag>>(m, "ConceptRoleFillersData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, OneOfTag>>(m, "ConceptOneOfData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::dl::BaseFamilyTag, NominalTag>>(m, "ConceptNominalData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, UniversalTag>>(m, "RoleUniversalData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, UnionTag>>(m, "RoleUnionData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, ComplementTag>>(m, "RoleComplementData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, InverseTag>>(m, "RoleInverseData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, CompositionTag>>(m, "RoleCompositionData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::dl::BaseFamilyTag, IdentityTag>>(m, "RoleIdentityData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicGoalStaticData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicGoalFluentData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicGoalDerivedData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::dl::BaseFamilyTag, NonemptyTag>>(m, "BooleanNonemptyData");
    bind_data<runir::kr::dl::grammar::Numerical<runir::kr::dl::BaseFamilyTag, CountTag>>(m, "NumericalCountData");
    bind_data<runir::kr::dl::grammar::Numerical<runir::kr::dl::BaseFamilyTag, DistanceTag>>(m, "NumericalDistanceData");
    bind_data<runir::kr::dl::grammar::Constructor<runir::kr::dl::BaseFamilyTag, ConceptTag>>(m, "ConceptData");
    bind_data<runir::kr::dl::grammar::Constructor<runir::kr::dl::BaseFamilyTag, RoleTag>>(m, "RoleData");
    bind_data<runir::kr::dl::grammar::Constructor<runir::kr::dl::BaseFamilyTag, BooleanTag>>(m, "BooleanData");
    bind_data<runir::kr::dl::grammar::Constructor<runir::kr::dl::BaseFamilyTag, NumericalTag>>(m, "NumericalData");
    bind_data<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BaseFamilyTag, ConceptTag>>(m, "ConceptNonTerminalData");
    bind_data<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BaseFamilyTag, RoleTag>>(m, "RoleNonTerminalData");
    bind_data<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BaseFamilyTag, BooleanTag>>(m, "BooleanNonTerminalData");
    bind_data<runir::kr::dl::grammar::NonTerminal<runir::kr::dl::BaseFamilyTag, NumericalTag>>(m, "NumericalNonTerminalData");
    bind_data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, ConceptTag>>(m, "ConceptConstructorOrNonTerminalData");
    bind_data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, RoleTag>>(m, "RoleConstructorOrNonTerminalData");
    bind_data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, BooleanTag>>(m, "BooleanConstructorOrNonTerminalData");
    bind_data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, NumericalTag>>(m, "NumericalConstructorOrNonTerminalData");
}

}  // namespace runir::kr::dl
