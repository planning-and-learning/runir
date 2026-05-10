#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/knowledge_representation/dl/grammar/constructor_repository.hpp>
#include <runir/knowledge_representation/dl/grammar/formatter.hpp>
#include <runir/knowledge_representation/dl/grammar/views.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

namespace grammar = runir::kr::dl::grammar;

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
    if constexpr (requires(const View& view) { view.get_name(); })
        cls.def("get_name", &View::get_name);
    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
}

}  // namespace

void bind_grammar_views(nb::module_& m)
{
    bind_view<grammar::Constructor<ConceptTag>, grammar::ConstructorRepository>(m, "Concept");
    bind_view<grammar::Constructor<RoleTag>, grammar::ConstructorRepository>(m, "Role");
    bind_view<grammar::Constructor<BooleanTag>, grammar::ConstructorRepository>(m, "Boolean");
    bind_view<grammar::Constructor<NumericalTag>, grammar::ConstructorRepository>(m, "Numerical");
    bind_view<grammar::Concept<BotTag>, grammar::ConstructorRepository>(m, "ConceptBot");
    bind_view<grammar::Concept<TopTag>, grammar::ConstructorRepository>(m, "ConceptTop");
    bind_view<grammar::Concept<AtomicStateTag<tyr::formalism::StaticTag>>, grammar::ConstructorRepository>(m, "ConceptAtomicStateStatic");
    bind_view<grammar::Concept<AtomicStateTag<tyr::formalism::FluentTag>>, grammar::ConstructorRepository>(m, "ConceptAtomicStateFluent");
    bind_view<grammar::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>, grammar::ConstructorRepository>(m, "ConceptAtomicStateDerived");
    bind_view<grammar::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>, grammar::ConstructorRepository>(m, "ConceptAtomicGoalStatic");
    bind_view<grammar::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>, grammar::ConstructorRepository>(m, "ConceptAtomicGoalFluent");
    bind_view<grammar::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>, grammar::ConstructorRepository>(m, "ConceptAtomicGoalDerived");
    bind_view<grammar::Concept<IntersectionTag>, grammar::ConstructorRepository>(m, "ConceptIntersection");
    bind_view<grammar::Concept<UnionTag>, grammar::ConstructorRepository>(m, "ConceptUnion");
    bind_view<grammar::Concept<NegationTag>, grammar::ConstructorRepository>(m, "ConceptNegation");
    bind_view<grammar::Concept<ValueRestrictionTag>, grammar::ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<grammar::Concept<ExistentialQuantificationTag>, grammar::ConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<grammar::Concept<RoleValueMapContainmentTag>, grammar::ConstructorRepository>(m, "ConceptRoleValueMapContainment");
    bind_view<grammar::Concept<RoleValueMapEqualityTag>, grammar::ConstructorRepository>(m, "ConceptRoleValueMapEquality");
    bind_view<grammar::Concept<NominalTag>, grammar::ConstructorRepository>(m, "ConceptNominal");
    bind_view<grammar::Role<UniversalTag>, grammar::ConstructorRepository>(m, "RoleUniversal");
    bind_view<grammar::Role<AtomicStateTag<tyr::formalism::StaticTag>>, grammar::ConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<grammar::Role<AtomicStateTag<tyr::formalism::FluentTag>>, grammar::ConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<grammar::Role<AtomicStateTag<tyr::formalism::DerivedTag>>, grammar::ConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<grammar::Role<AtomicGoalTag<tyr::formalism::StaticTag>>, grammar::ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<grammar::Role<AtomicGoalTag<tyr::formalism::FluentTag>>, grammar::ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<grammar::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>, grammar::ConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<grammar::Role<IntersectionTag>, grammar::ConstructorRepository>(m, "RoleIntersection");
    bind_view<grammar::Role<UnionTag>, grammar::ConstructorRepository>(m, "RoleUnion");
    bind_view<grammar::Role<ComplementTag>, grammar::ConstructorRepository>(m, "RoleComplement");
    bind_view<grammar::Role<InverseTag>, grammar::ConstructorRepository>(m, "RoleInverse");
    bind_view<grammar::Role<CompositionTag>, grammar::ConstructorRepository>(m, "RoleComposition");
    bind_view<grammar::Role<TransitiveClosureTag>, grammar::ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<grammar::Role<ReflexiveTransitiveClosureTag>, grammar::ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<grammar::Role<RestrictionTag>, grammar::ConstructorRepository>(m, "RoleRestriction");
    bind_view<grammar::Role<IdentityTag>, grammar::ConstructorRepository>(m, "RoleIdentity");
    bind_view<grammar::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>, grammar::ConstructorRepository>(m, "BooleanAtomicStateStatic");
    bind_view<grammar::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>, grammar::ConstructorRepository>(m, "BooleanAtomicStateFluent");
    bind_view<grammar::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>, grammar::ConstructorRepository>(m, "BooleanAtomicStateDerived");
    bind_view<grammar::Boolean<NonemptyTag>, grammar::ConstructorRepository>(m, "BooleanNonempty");
    bind_view<grammar::Numerical<CountTag>, grammar::ConstructorRepository>(m, "NumericalCount");
    bind_view<grammar::Numerical<DistanceTag>, grammar::ConstructorRepository>(m, "NumericalDistance");
}

}  // namespace runir::kr::dl
