#include "../data_bindings.hpp"
#include "bindings.hpp"

#include <concepts>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/role_view.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl
{
namespace
{

template<typename Tag>
void bind_role_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<Role<Tag>>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    ygg::add_comparison(cls);
    python::def_role_data_constructors<Tag,
                                       ygg::Index<runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>,
                                       ygg::Index<runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>,
                                       runir::kr::dl::ExtConstructorRepository>(cls);

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def_rw("predicate", &Data::predicate).def_rw("polarity", &Data::polarity);
    else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag> || std::same_as<Tag, CompositionTag>
                       || std::same_as<Tag, RestrictionTag>)
        cls.def_rw("lhs", &Data::lhs).def_rw("rhs", &Data::rhs);
    else if constexpr (std::same_as<Tag, RegisterTag> || std::same_as<Tag, ArgumentTag<RoleTag>>)
        cls.def_rw("reference", &Data::reference);
    else if constexpr (!std::same_as<Tag, UniversalTag>)
        cls.def_rw("arg", &Data::arg);
}

template<typename Tag>
void bind_role_view(nb::module_& m, const char* name)
{
    using Type = Role<Tag>;
    using View = ygg::View<ygg::Index<Type>, ExtConstructorRepository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    cls.def("syntactic_complexity", [](View view) { return semantics::syntactic_complexity(view); });

    if constexpr (is_atomic_state_tag_v<Tag> || is_atomic_goal_tag_v<Tag>)
        cls.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>()).def("get_polarity", &View::get_polarity);
    else if constexpr (std::same_as<Tag, IntersectionTag> || std::same_as<Tag, UnionTag> || std::same_as<Tag, CompositionTag>
                       || std::same_as<Tag, RestrictionTag>)
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>()).def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, RegisterTag>)
        cls.def("get_register", &View::get_register, nb::keep_alive<0, 1>());
    else if constexpr (std::same_as<Tag, ArgumentTag<RoleTag>>)
        cls.def("get_argument", &View::get_argument, nb::keep_alive<0, 1>());
    else if constexpr (!std::same_as<Tag, UniversalTag>)
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
}

}  // namespace

void bind_semantics_role(nb::module_& m)
{
    using Universal = Role<UniversalTag>;
    using StaticState = Role<AtomicStateTag<tyr::formalism::StaticTag>>;
    using FluentState = Role<AtomicStateTag<tyr::formalism::FluentTag>>;
    using DerivedState = Role<AtomicStateTag<tyr::formalism::DerivedTag>>;
    using StaticGoal = Role<AtomicGoalTag<tyr::formalism::StaticTag>>;
    using FluentGoal = Role<AtomicGoalTag<tyr::formalism::FluentTag>>;
    using DerivedGoal = Role<AtomicGoalTag<tyr::formalism::DerivedTag>>;
    using Intersection = Role<IntersectionTag>;
    using Union = Role<UnionTag>;
    using Complement = Role<ComplementTag>;
    using Inverse = Role<InverseTag>;
    using Composition = Role<CompositionTag>;
    using TransitiveClosure = Role<TransitiveClosureTag>;
    using ReflexiveTransitiveClosure = Role<ReflexiveTransitiveClosureTag>;
    using Restriction = Role<RestrictionTag>;
    using Identity = Role<IdentityTag>;
    using RegisterReference = Role<RegisterTag>;
    using ArgumentReference = Role<ArgumentTag<RoleTag>>;

    ygg::bind_index<ygg::Index<Universal>>(m, "RoleUniversalIndex");
    ygg::bind_index<ygg::Index<StaticState>>(m, "RoleAtomicStateStaticIndex");
    ygg::bind_index<ygg::Index<FluentState>>(m, "RoleAtomicStateFluentIndex");
    ygg::bind_index<ygg::Index<DerivedState>>(m, "RoleAtomicStateDerivedIndex");
    ygg::bind_index<ygg::Index<StaticGoal>>(m, "RoleAtomicGoalStaticIndex");
    ygg::bind_index<ygg::Index<FluentGoal>>(m, "RoleAtomicGoalFluentIndex");
    ygg::bind_index<ygg::Index<DerivedGoal>>(m, "RoleAtomicGoalDerivedIndex");
    ygg::bind_index<ygg::Index<Intersection>>(m, "RoleIntersectionIndex");
    ygg::bind_index<ygg::Index<Union>>(m, "RoleUnionIndex");
    ygg::bind_index<ygg::Index<Complement>>(m, "RoleComplementIndex");
    ygg::bind_index<ygg::Index<Inverse>>(m, "RoleInverseIndex");
    ygg::bind_index<ygg::Index<Composition>>(m, "RoleCompositionIndex");
    ygg::bind_index<ygg::Index<TransitiveClosure>>(m, "RoleTransitiveClosureIndex");
    ygg::bind_index<ygg::Index<ReflexiveTransitiveClosure>>(m, "RoleReflexiveTransitiveClosureIndex");
    ygg::bind_index<ygg::Index<Restriction>>(m, "RoleRestrictionIndex");
    ygg::bind_index<ygg::Index<Identity>>(m, "RoleIdentityIndex");
    ygg::bind_index<ygg::Index<RegisterReference>>(m, "RoleRegisterIndex");
    ygg::bind_index<ygg::Index<ArgumentReference>>(m, "RoleArgumentIndex");

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
    bind_role_data<RegisterTag>(m, "RoleRegisterData");
    bind_role_data<ArgumentTag<RoleTag>>(m, "RoleArgumentData");

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
    bind_role_view<RegisterTag>(m, "RoleRegister");
    bind_role_view<ArgumentTag<RoleTag>>(m, "RoleArgument");
}

}  // namespace runir::kr::dl
