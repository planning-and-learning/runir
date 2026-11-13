#ifndef PYRUNIR_KR_DL_RECORD_BINDINGS_HPP_
#define PYRUNIR_KR_DL_RECORD_BINDINGS_HPP_

#include "module.hpp"
#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/formatter.hpp>
#include <runir/kr/dl/record_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <string>
#include <tyr/formalism/planning/repository.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::dl::python
{

// The description-logic constructor records (Concept/Role/Boolean/Numerical<Family, Tag>) of one family, bound under
// the same Python names in each family's module: <Name>Index, <Name>Data and the view <Name>.

using runir::kr::python::def_fields;

/// A view whose record's syntactic complexity is defined (the semantic family's).
template<typename View>
concept MeasuredView = requires(const View& view) { semantics::syntactic_complexity(view); };

/// A family that has a record of Record and Tag.
template<template<typename, typename> typename Record, typename F, typename Tag>
concept FamilyRecord = requires { typename Record<F, Tag>; };

/// Binds Record<F, Tag> under name, if Tag is one of F's constructors.
template<template<typename, typename> typename Record, typename Tag, typename F, typename Repository>
void bind_record(nb::module_& m, const std::string& name)
{
    if constexpr (FamilyRecord<Record, F, Tag>)
    {
        using Type = Record<F, Tag>;
        using Data = urd::data<Type>;
        using View = urd::index_view<Type, Repository>;

        urd::python::bind_index<urd::index<Type>>(m, name + "Index");

        auto data = nb::class_<Data>(m, (name + "Data").c_str()).def(nb::init<>()).def_rw("index", &Data::index);
        urd::python::add_comparison(data);
        def_fields<Repository>(data);

        auto view = nb::class_<View>(m, name.c_str()).def("index", &View::index);
        ygg::add_print(view);
        urd::python::add_comparison(view);
        urd::python::add_hash(view);
        urd::python::bind_field_getters(view);
        if constexpr (MeasuredView<View>)
            view.def("syntactic_complexity", [](View v) { return semantics::syntactic_complexity(v); });
    }
}

/// The records of one constructor category: a family with heavy records binds the categories in several sources.
template<typename F, typename Repository>
void bind_records(nb::module_& m, ConceptTag)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    bind_record<Concept, BotTag, F, Repository>(m, "ConceptBot");
    bind_record<Concept, TopTag, F, Repository>(m, "ConceptTop");
    bind_record<Concept, AtomicStateTag<StaticTag>, F, Repository>(m, "ConceptAtomicStateStatic");
    bind_record<Concept, AtomicStateTag<FluentTag>, F, Repository>(m, "ConceptAtomicStateFluent");
    bind_record<Concept, AtomicStateTag<DerivedTag>, F, Repository>(m, "ConceptAtomicStateDerived");
    bind_record<Concept, AtomicGoalTag<StaticTag>, F, Repository>(m, "ConceptAtomicGoalStatic");
    bind_record<Concept, AtomicGoalTag<FluentTag>, F, Repository>(m, "ConceptAtomicGoalFluent");
    bind_record<Concept, AtomicGoalTag<DerivedTag>, F, Repository>(m, "ConceptAtomicGoalDerived");
    bind_record<Concept, IntersectionTag, F, Repository>(m, "ConceptIntersection");
    bind_record<Concept, UnionTag, F, Repository>(m, "ConceptUnion");
    bind_record<Concept, NegationTag, F, Repository>(m, "ConceptNegation");
    bind_record<Concept, ValueRestrictionTag, F, Repository>(m, "ConceptValueRestriction");
    bind_record<Concept, ExistentialQuantificationTag, F, Repository>(m, "ConceptExistentialQuantification");
    bind_record<Concept, AtLeastNumberRestrictionTag, F, Repository>(m, "ConceptAtLeastNumberRestriction");
    bind_record<Concept, AtMostNumberRestrictionTag, F, Repository>(m, "ConceptAtMostNumberRestriction");
    bind_record<Concept, ExactNumberRestrictionTag, F, Repository>(m, "ConceptExactNumberRestriction");
    bind_record<Concept, QualifiedAtLeastNumberRestrictionTag, F, Repository>(m, "ConceptQualifiedAtLeastNumberRestriction");
    bind_record<Concept, QualifiedAtMostNumberRestrictionTag, F, Repository>(m, "ConceptQualifiedAtMostNumberRestriction");
    bind_record<Concept, QualifiedExactNumberRestrictionTag, F, Repository>(m, "ConceptQualifiedExactNumberRestriction");
    bind_record<Concept, RoleValueMapTag, F, Repository>(m, "ConceptRoleValueMap");
    bind_record<Concept, AgreementTag, F, Repository>(m, "ConceptAgreement");
    bind_record<Concept, RoleFillersTag, F, Repository>(m, "ConceptRoleFillers");
    bind_record<Concept, OneOfTag, F, Repository>(m, "ConceptOneOf");
    bind_record<Concept, NominalTag, F, Repository>(m, "ConceptNominal");
    bind_record<Concept, RegisterTag, F, Repository>(m, "ConceptRegister");
    bind_record<Concept, ArgumentTag<ConceptTag>, F, Repository>(m, "ConceptArgument");
}

template<typename F, typename Repository>
void bind_records(nb::module_& m, RoleTag)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    bind_record<Role, UniversalTag, F, Repository>(m, "RoleUniversal");
    bind_record<Role, AtomicStateTag<StaticTag>, F, Repository>(m, "RoleAtomicStateStatic");
    bind_record<Role, AtomicStateTag<FluentTag>, F, Repository>(m, "RoleAtomicStateFluent");
    bind_record<Role, AtomicStateTag<DerivedTag>, F, Repository>(m, "RoleAtomicStateDerived");
    bind_record<Role, AtomicGoalTag<StaticTag>, F, Repository>(m, "RoleAtomicGoalStatic");
    bind_record<Role, AtomicGoalTag<FluentTag>, F, Repository>(m, "RoleAtomicGoalFluent");
    bind_record<Role, AtomicGoalTag<DerivedTag>, F, Repository>(m, "RoleAtomicGoalDerived");
    bind_record<Role, IntersectionTag, F, Repository>(m, "RoleIntersection");
    bind_record<Role, UnionTag, F, Repository>(m, "RoleUnion");
    bind_record<Role, ComplementTag, F, Repository>(m, "RoleComplement");
    bind_record<Role, InverseTag, F, Repository>(m, "RoleInverse");
    bind_record<Role, CompositionTag, F, Repository>(m, "RoleComposition");
    bind_record<Role, TransitiveClosureTag, F, Repository>(m, "RoleTransitiveClosure");
    bind_record<Role, ReflexiveTransitiveClosureTag, F, Repository>(m, "RoleReflexiveTransitiveClosure");
    bind_record<Role, RestrictionTag, F, Repository>(m, "RoleRestriction");
    bind_record<Role, IdentityTag, F, Repository>(m, "RoleIdentity");
    bind_record<Role, RegisterTag, F, Repository>(m, "RoleRegister");
    bind_record<Role, ArgumentTag<RoleTag>, F, Repository>(m, "RoleArgument");
}

template<typename F, typename Repository>
void bind_records(nb::module_& m, BooleanTag)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    bind_record<Boolean, AtomicStateTag<StaticTag>, F, Repository>(m, "BooleanAtomicStateStatic");
    bind_record<Boolean, AtomicStateTag<FluentTag>, F, Repository>(m, "BooleanAtomicStateFluent");
    bind_record<Boolean, AtomicStateTag<DerivedTag>, F, Repository>(m, "BooleanAtomicStateDerived");
    bind_record<Boolean, AtomicGoalTag<StaticTag>, F, Repository>(m, "BooleanAtomicGoalStatic");
    bind_record<Boolean, AtomicGoalTag<FluentTag>, F, Repository>(m, "BooleanAtomicGoalFluent");
    bind_record<Boolean, AtomicGoalTag<DerivedTag>, F, Repository>(m, "BooleanAtomicGoalDerived");
    bind_record<Boolean, NonemptyTag, F, Repository>(m, "BooleanNonempty");
    bind_record<Boolean, ArgumentTag<BooleanTag>, F, Repository>(m, "BooleanArgument");
    bind_record<Boolean, EqTag<BooleanTag>, F, Repository>(m, "BooleanEq");
    bind_record<Boolean, NeqTag<BooleanTag>, F, Repository>(m, "BooleanNeq");
    bind_record<Boolean, LtTag<BooleanTag>, F, Repository>(m, "BooleanLt");
    bind_record<Boolean, LeTag<BooleanTag>, F, Repository>(m, "BooleanLe");
    bind_record<Boolean, GtTag<BooleanTag>, F, Repository>(m, "BooleanGt");
    bind_record<Boolean, GeTag<BooleanTag>, F, Repository>(m, "BooleanGe");
    bind_record<Boolean, EqTag<NumericalTag>, F, Repository>(m, "NumericalEq");
    bind_record<Boolean, NeqTag<NumericalTag>, F, Repository>(m, "NumericalNeq");
    bind_record<Boolean, LtTag<NumericalTag>, F, Repository>(m, "NumericalLt");
    bind_record<Boolean, LeTag<NumericalTag>, F, Repository>(m, "NumericalLe");
    bind_record<Boolean, GtTag<NumericalTag>, F, Repository>(m, "NumericalGt");
    bind_record<Boolean, GeTag<NumericalTag>, F, Repository>(m, "NumericalGe");
    bind_record<Boolean, BooleanConstantTag, F, Repository>(m, "BooleanConstant");
    bind_record<Boolean, AndTag, F, Repository>(m, "BooleanAnd");
    bind_record<Boolean, OrTag, F, Repository>(m, "BooleanOr");
    bind_record<Boolean, NotTag, F, Repository>(m, "BooleanNot");
}

template<typename F, typename Repository>
void bind_records(nb::module_& m, NumericalTag)
{
    bind_record<Numerical, CountTag, F, Repository>(m, "NumericalCount");
    bind_record<Numerical, DistanceTag, F, Repository>(m, "NumericalDistance");
    bind_record<Numerical, ArgumentTag<NumericalTag>, F, Repository>(m, "NumericalArgument");
    bind_record<Numerical, NumericalConstantTag, F, Repository>(m, "NumericalConstant");
    bind_record<Numerical, AddTag, F, Repository>(m, "NumericalAdd");
    bind_record<Numerical, SubTag, F, Repository>(m, "NumericalSub");
    bind_record<Numerical, MulTag, F, Repository>(m, "NumericalMul");
    bind_record<Numerical, DivTag, F, Repository>(m, "NumericalDiv");
    bind_record<Numerical, MinTag, F, Repository>(m, "NumericalMin");
    bind_record<Numerical, MaxTag, F, Repository>(m, "NumericalMax");
}

/// The records of every constructor category.
template<typename F, typename Repository>
void bind_records(nb::module_& m)
{
    bind_records<F, Repository>(m, ConceptTag {});
    bind_records<F, Repository>(m, RoleTag {});
    bind_records<F, Repository>(m, BooleanTag {});
    bind_records<F, Repository>(m, NumericalTag {});
}

}  // namespace runir::kr::dl::python

#endif
