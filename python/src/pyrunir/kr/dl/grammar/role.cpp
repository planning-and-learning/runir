#include "../data_bindings.hpp"
#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/role_view.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::dl
{
namespace
{

template<typename Tag>
void bind_role_data(nb::module_& m, const char* name)
{
    using Data = urd::data<runir::kr::dl::grammar::Role<Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    urd::python::add_comparison(cls);
    python::def_role_data_constructors<Tag,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorRepository>(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def_rw("predicate", &Data::predicate).def_rw("polarity", &Data::polarity);
    else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag> || std::same_as<Tag, CompositionTag>
                       || std::same_as<Tag, RestrictionTag>)
        cls.def_rw("lhs", &Data::lhs).def_rw("rhs", &Data::rhs);
    else if constexpr (!std::same_as<Tag, UniversalTag>)
        cls.def_rw("arg", &Data::arg);
}

template<typename Tag>
void bind_role_view(nb::module_& m, const char* name)
{
    using Type = runir::kr::dl::grammar::Role<Tag>;
    using View = urd::index_view<Type, runir::kr::dl::grammar::ConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::index);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>()).def("get_polarity", &View::get_polarity);
    else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag> || std::same_as<Tag, CompositionTag>
                       || std::same_as<Tag, RestrictionTag>)
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>()).def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    else if constexpr (!std::same_as<Tag, UniversalTag>)
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_grammar_role(nb::module_& m)
{
    using Universal = runir::kr::dl::grammar::Role<UniversalTag>;
    using StaticState = runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::StaticTag>>;
    using FluentState = runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::FluentTag>>;
    using DerivedState = runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::DerivedTag>>;
    using StaticGoal = runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::StaticTag>>;
    using FluentGoal = runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::FluentTag>>;
    using DerivedGoal = runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>;
    using Intersection = runir::kr::dl::grammar::Role<IntersectionTag>;
    using Union = runir::kr::dl::grammar::Role<UnionTag>;
    using Complement = runir::kr::dl::grammar::Role<ComplementTag>;
    using Inverse = runir::kr::dl::grammar::Role<InverseTag>;
    using Composition = runir::kr::dl::grammar::Role<CompositionTag>;
    using TransitiveClosure = runir::kr::dl::grammar::Role<TransitiveClosureTag>;
    using ReflexiveTransitiveClosure = runir::kr::dl::grammar::Role<ReflexiveTransitiveClosureTag>;
    using Restriction = runir::kr::dl::grammar::Role<RestrictionTag>;
    using Identity = runir::kr::dl::grammar::Role<IdentityTag>;

    urd::python::bind_index<urd::index<Universal>>(m, "RoleUniversalIndex");
    urd::python::bind_index<urd::index<StaticState>>(m, "RoleAtomicStateStaticIndex");
    urd::python::bind_index<urd::index<FluentState>>(m, "RoleAtomicStateFluentIndex");
    urd::python::bind_index<urd::index<DerivedState>>(m, "RoleAtomicStateDerivedIndex");
    urd::python::bind_index<urd::index<StaticGoal>>(m, "RoleAtomicGoalStaticIndex");
    urd::python::bind_index<urd::index<FluentGoal>>(m, "RoleAtomicGoalFluentIndex");
    urd::python::bind_index<urd::index<DerivedGoal>>(m, "RoleAtomicGoalDerivedIndex");
    urd::python::bind_index<urd::index<Intersection>>(m, "RoleIntersectionIndex");
    urd::python::bind_index<urd::index<Union>>(m, "RoleUnionIndex");
    urd::python::bind_index<urd::index<Complement>>(m, "RoleComplementIndex");
    urd::python::bind_index<urd::index<Inverse>>(m, "RoleInverseIndex");
    urd::python::bind_index<urd::index<Composition>>(m, "RoleCompositionIndex");
    urd::python::bind_index<urd::index<TransitiveClosure>>(m, "RoleTransitiveClosureIndex");
    urd::python::bind_index<urd::index<ReflexiveTransitiveClosure>>(m, "RoleReflexiveTransitiveClosureIndex");
    urd::python::bind_index<urd::index<Restriction>>(m, "RoleRestrictionIndex");
    urd::python::bind_index<urd::index<Identity>>(m, "RoleIdentityIndex");

    bind_role_data<UniversalTag>(m, "RoleUniversalData");
    bind_role_data<AtomicStateTag<tyr::formalism::StaticTag>>(m, "RoleAtomicStateStaticData");
    bind_role_data<AtomicStateTag<tyr::formalism::FluentTag>>(m, "RoleAtomicStateFluentData");
    bind_role_data<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "RoleAtomicStateDerivedData");
    bind_role_data<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "RoleAtomicGoalStaticData");
    bind_role_data<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "RoleAtomicGoalFluentData");
    bind_role_data<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "RoleAtomicGoalDerivedData");
    bind_role_data<IntersectionTag>(m, "RoleIntersectionData");
    bind_role_data<UnionTag>(m, "RoleUnionData");
    bind_role_data<ComplementTag>(m, "RoleComplementData");
    bind_role_data<InverseTag>(m, "RoleInverseData");
    bind_role_data<CompositionTag>(m, "RoleCompositionData");
    bind_role_data<TransitiveClosureTag>(m, "RoleTransitiveClosureData");
    bind_role_data<ReflexiveTransitiveClosureTag>(m, "RoleReflexiveTransitiveClosureData");
    bind_role_data<RestrictionTag>(m, "RoleRestrictionData");
    bind_role_data<IdentityTag>(m, "RoleIdentityData");

    bind_role_view<UniversalTag>(m, "RoleUniversal");
    bind_role_view<AtomicStateTag<tyr::formalism::StaticTag>>(m, "RoleAtomicStateStatic");
    bind_role_view<AtomicStateTag<tyr::formalism::FluentTag>>(m, "RoleAtomicStateFluent");
    bind_role_view<AtomicStateTag<tyr::formalism::DerivedTag>>(m, "RoleAtomicStateDerived");
    bind_role_view<AtomicGoalTag<tyr::formalism::StaticTag>>(m, "RoleAtomicGoalStatic");
    bind_role_view<AtomicGoalTag<tyr::formalism::FluentTag>>(m, "RoleAtomicGoalFluent");
    bind_role_view<AtomicGoalTag<tyr::formalism::DerivedTag>>(m, "RoleAtomicGoalDerived");
    bind_role_view<IntersectionTag>(m, "RoleIntersection");
    bind_role_view<UnionTag>(m, "RoleUnion");
    bind_role_view<ComplementTag>(m, "RoleComplement");
    bind_role_view<InverseTag>(m, "RoleInverse");
    bind_role_view<CompositionTag>(m, "RoleComposition");
    bind_role_view<TransitiveClosureTag>(m, "RoleTransitiveClosure");
    bind_role_view<ReflexiveTransitiveClosureTag>(m, "RoleReflexiveTransitiveClosure");
    bind_role_view<RestrictionTag>(m, "RoleRestriction");
    bind_role_view<IdentityTag>(m, "RoleIdentity");
}

}  // namespace runir::kr::dl
