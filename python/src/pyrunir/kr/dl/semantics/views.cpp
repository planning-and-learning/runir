#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/knowledge_representation/dl/repository.hpp>
#include <runir/knowledge_representation/dl/semantics/formatter.hpp>
#include <runir/knowledge_representation/dl/semantics/views.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

namespace
{

template<typename T, typename Repository>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = tyr::View<tyr::Index<T>, Repository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    tyr::add_print(cls);
    tyr::add_hash(cls);

    if constexpr (requires(const View& view) { view.get_arg(); })
        cls.def("get_arg", &View::get_arg);
    if constexpr (requires(const View& view) { view.get_lhs(); })
        cls.def("get_lhs", &View::get_lhs);
    if constexpr (requires(const View& view) { view.get_mid(); })
        cls.def("get_mid", &View::get_mid);
    if constexpr (requires(const View& view) { view.get_rhs(); })
        cls.def("get_rhs", &View::get_rhs);
    if constexpr (requires(const View& view) { view.get_predicate(); })
        cls.def("get_predicate", &View::get_predicate);
    if constexpr (requires(const View& view) { view.get_polarity(); })
        cls.def("get_polarity", &View::get_polarity);
    if constexpr (requires(const View& view) { view.get_object(); })
        cls.def("get_object", &View::get_object);
}

}  // namespace

void bind_semantics_views(nb::module_& m)
{
    bind_view<Constructor<ConceptTag>, ConstructorRepository>(m, "Concept");
    bind_view<Constructor<RoleTag>, ConstructorRepository>(m, "Role");
    bind_view<Constructor<BooleanTag>, ConstructorRepository>(m, "Boolean");
    bind_view<Constructor<NumericalTag>, ConstructorRepository>(m, "Numerical");

    bind_view<Concept<BotTag>, ConstructorRepository>(m, "ConceptBot");
    bind_view<Concept<TopTag>, ConstructorRepository>(m, "ConceptTop");
    bind_view<Concept<AtomicStateTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "ConceptAtomicStateStatic");
    bind_view<Concept<AtomicStateTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "ConceptAtomicStateFluent");
    bind_view<Concept<AtomicStateTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "ConceptAtomicStateDerived");
    bind_view<Concept<AtomicGoalTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "ConceptAtomicGoalStatic");
    bind_view<Concept<AtomicGoalTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "ConceptAtomicGoalFluent");
    bind_view<Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "ConceptAtomicGoalDerived");
    bind_view<Concept<IntersectionTag>, ConstructorRepository>(m, "ConceptIntersection");
    bind_view<Concept<UnionTag>, ConstructorRepository>(m, "ConceptUnion");
    bind_view<Concept<NegationTag>, ConstructorRepository>(m, "ConceptNegation");
    bind_view<Concept<ValueRestrictionTag>, ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<Concept<ExistentialQuantificationTag>, ConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<Concept<RoleValueMapContainmentTag>, ConstructorRepository>(m, "ConceptRoleValueMapContainment");
    bind_view<Concept<RoleValueMapEqualityTag>, ConstructorRepository>(m, "ConceptRoleValueMapEquality");
    bind_view<Concept<NominalTag>, ConstructorRepository>(m, "ConceptNominal");

    bind_view<Role<UniversalTag>, ConstructorRepository>(m, "RoleUniversal");
    bind_view<Role<AtomicStateTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<Role<AtomicStateTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<Role<AtomicStateTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<Role<AtomicGoalTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<Role<AtomicGoalTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<Role<AtomicGoalTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<Role<IntersectionTag>, ConstructorRepository>(m, "RoleIntersection");
    bind_view<Role<UnionTag>, ConstructorRepository>(m, "RoleUnion");
    bind_view<Role<ComplementTag>, ConstructorRepository>(m, "RoleComplement");
    bind_view<Role<InverseTag>, ConstructorRepository>(m, "RoleInverse");
    bind_view<Role<CompositionTag>, ConstructorRepository>(m, "RoleComposition");
    bind_view<Role<TransitiveClosureTag>, ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<Role<ReflexiveTransitiveClosureTag>, ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<Role<RestrictionTag>, ConstructorRepository>(m, "RoleRestriction");
    bind_view<Role<IdentityTag>, ConstructorRepository>(m, "RoleIdentity");

    bind_view<Boolean<AtomicStateTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "BooleanAtomicStateStatic");
    bind_view<Boolean<AtomicStateTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "BooleanAtomicStateFluent");
    bind_view<Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "BooleanAtomicStateDerived");
    bind_view<Boolean<NonemptyTag>, ConstructorRepository>(m, "BooleanNonempty");

    bind_view<Numerical<CountTag>, ConstructorRepository>(m, "NumericalCount");
    bind_view<Numerical<DistanceTag>, ConstructorRepository>(m, "NumericalDistance");
}

}  // namespace runir::kr::dl
