#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/base/datas.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::kr::dl::base
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
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, BotTag>>(m, "ConceptBotData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, TopTag>>(m, "ConceptTopData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "ConceptAtomicGoalStaticData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "ConceptAtomicGoalFluentData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "ConceptAtomicGoalDerivedData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, IntersectionTag>>(m, "ConceptIntersectionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, UnionTag>>(m, "ConceptUnionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, NegationTag>>(m, "ConceptNegationData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, ValueRestrictionTag>>(m, "ConceptValueRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, ExistentialQuantificationTag>>(m, "ConceptExistentialQuantificationData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AtLeastNumberRestrictionTag>>(m, "ConceptAtLeastNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AtMostNumberRestrictionTag>>(m, "ConceptAtMostNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, ExactNumberRestrictionTag>>(m, "ConceptExactNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>>(m,
                                                                                                               "ConceptQualifiedAtLeastNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>>(m, "ConceptQualifiedAtMostNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, QualifiedExactNumberRestrictionTag>>(m, "ConceptQualifiedExactNumberRestrictionData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, RoleValueMapTag>>(m, "ConceptRoleValueMapData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, AgreementTag>>(m, "ConceptAgreementData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, RoleFillersTag>>(m, "ConceptRoleFillersData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, OneOfTag>>(m, "ConceptOneOfData");
    bind_data<runir::kr::dl::grammar::Concept<runir::kr::BaseFamilyTag, NominalTag>>(m, "ConceptNominalData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, UniversalTag>>(m, "RoleUniversalData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "RoleAtomicGoalStaticData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "RoleAtomicGoalFluentData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "RoleAtomicGoalDerivedData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, IntersectionTag>>(m, "RoleIntersectionData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, UnionTag>>(m, "RoleUnionData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, ComplementTag>>(m, "RoleComplementData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, InverseTag>>(m, "RoleInverseData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, CompositionTag>>(m, "RoleCompositionData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, TransitiveClosureTag>>(m, "RoleTransitiveClosureData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, ReflexiveTransitiveClosureTag>>(m, "RoleReflexiveTransitiveClosureData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, RestrictionTag>>(m, "RoleRestrictionData");
    bind_data<runir::kr::dl::grammar::Role<runir::kr::BaseFamilyTag, IdentityTag>>(m, "RoleIdentityData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicStateStaticData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicStateFluentData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicStateDerivedData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>>(m, "BooleanAtomicGoalStaticData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>>(m, "BooleanAtomicGoalFluentData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>>(m, "BooleanAtomicGoalDerivedData");
    bind_data<runir::kr::dl::grammar::Boolean<runir::kr::BaseFamilyTag, NonemptyTag>>(m, "BooleanNonemptyData");
    bind_data<runir::kr::dl::grammar::Numerical<runir::kr::BaseFamilyTag, CountTag>>(m, "NumericalCountData");
    bind_data<runir::kr::dl::grammar::Numerical<runir::kr::BaseFamilyTag, DistanceTag>>(m, "NumericalDistanceData");
    bind_data<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, ConceptTag>>(m, "ConceptData");
    bind_data<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, RoleTag>>(m, "RoleData");
    bind_data<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, BooleanTag>>(m, "BooleanData");
    bind_data<runir::kr::dl::grammar::Constructor<runir::kr::BaseFamilyTag, NumericalTag>>(m, "NumericalData");
    bind_data<runir::kr::dl::grammar::NonTerminal<runir::kr::BaseFamilyTag, ConceptTag>>(m, "ConceptNonTerminalData");
    bind_data<runir::kr::dl::grammar::NonTerminal<runir::kr::BaseFamilyTag, RoleTag>>(m, "RoleNonTerminalData");
    bind_data<runir::kr::dl::grammar::NonTerminal<runir::kr::BaseFamilyTag, BooleanTag>>(m, "BooleanNonTerminalData");
    bind_data<runir::kr::dl::grammar::NonTerminal<runir::kr::BaseFamilyTag, NumericalTag>>(m, "NumericalNonTerminalData");
    bind_data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, ConceptTag>>(m, "ConceptConstructorOrNonTerminalData");
    bind_data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, RoleTag>>(m, "RoleConstructorOrNonTerminalData");
    bind_data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, BooleanTag>>(m, "BooleanConstructorOrNonTerminalData");
    bind_data<runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::BaseFamilyTag, NumericalTag>>(m, "NumericalConstructorOrNonTerminalData");
}

}  // namespace runir::kr::dl::base
