#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/knowledge_representation/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/formatter.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/views.hpp>
#include <tyr/common/python/bindings.hpp>

namespace runir::kr::dl
{

namespace cnf = runir::kr::dl::cnf_grammar;

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

void bind_cnf_grammar_views(nb::module_& m)
{
    bind_view<cnf::Constructor<ConceptTag>, cnf::ConstructorRepository>(m, "Concept");
    bind_view<cnf::Constructor<RoleTag>, cnf::ConstructorRepository>(m, "Role");
    bind_view<cnf::Constructor<BooleanTag>, cnf::ConstructorRepository>(m, "Boolean");
    bind_view<cnf::Constructor<NumericalTag>, cnf::ConstructorRepository>(m, "Numerical");
    bind_view<cnf::Concept<BotTag>, cnf::ConstructorRepository>(m, "ConceptBot");
    bind_view<cnf::Concept<TopTag>, cnf::ConstructorRepository>(m, "ConceptTop");
    bind_view<cnf::Concept<AtomicStateTag<tyr::formalism::StaticTag>>, cnf::ConstructorRepository>(m, "ConceptAtomicStateStatic");
    bind_view<cnf::Concept<AtomicStateTag<tyr::formalism::FluentTag>>, cnf::ConstructorRepository>(m, "ConceptAtomicStateFluent");
    bind_view<cnf::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>, cnf::ConstructorRepository>(m, "ConceptAtomicStateDerived");
    bind_view<cnf::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>, cnf::ConstructorRepository>(m, "ConceptAtomicGoalStatic");
    bind_view<cnf::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>, cnf::ConstructorRepository>(m, "ConceptAtomicGoalFluent");
    bind_view<cnf::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>, cnf::ConstructorRepository>(m, "ConceptAtomicGoalDerived");
    bind_view<cnf::Concept<IntersectionTag>, cnf::ConstructorRepository>(m, "ConceptIntersection");
    bind_view<cnf::Concept<UnionTag>, cnf::ConstructorRepository>(m, "ConceptUnion");
    bind_view<cnf::Concept<NegationTag>, cnf::ConstructorRepository>(m, "ConceptNegation");
    bind_view<cnf::Concept<ValueRestrictionTag>, cnf::ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<cnf::Concept<ExistentialQuantificationTag>, cnf::ConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<cnf::Concept<RoleValueMapContainmentTag>, cnf::ConstructorRepository>(m, "ConceptRoleValueMapContainment");
    bind_view<cnf::Concept<RoleValueMapEqualityTag>, cnf::ConstructorRepository>(m, "ConceptRoleValueMapEquality");
    bind_view<cnf::Concept<NominalTag>, cnf::ConstructorRepository>(m, "ConceptNominal");
    bind_view<cnf::Role<UniversalTag>, cnf::ConstructorRepository>(m, "RoleUniversal");
    bind_view<cnf::Role<AtomicStateTag<tyr::formalism::StaticTag>>, cnf::ConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<cnf::Role<AtomicStateTag<tyr::formalism::FluentTag>>, cnf::ConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<cnf::Role<AtomicStateTag<tyr::formalism::DerivedTag>>, cnf::ConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<cnf::Role<AtomicGoalTag<tyr::formalism::StaticTag>>, cnf::ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<cnf::Role<AtomicGoalTag<tyr::formalism::FluentTag>>, cnf::ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<cnf::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>, cnf::ConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<cnf::Role<IntersectionTag>, cnf::ConstructorRepository>(m, "RoleIntersection");
    bind_view<cnf::Role<UnionTag>, cnf::ConstructorRepository>(m, "RoleUnion");
    bind_view<cnf::Role<ComplementTag>, cnf::ConstructorRepository>(m, "RoleComplement");
    bind_view<cnf::Role<InverseTag>, cnf::ConstructorRepository>(m, "RoleInverse");
    bind_view<cnf::Role<CompositionTag>, cnf::ConstructorRepository>(m, "RoleComposition");
    bind_view<cnf::Role<TransitiveClosureTag>, cnf::ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<cnf::Role<ReflexiveTransitiveClosureTag>, cnf::ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<cnf::Role<RestrictionTag>, cnf::ConstructorRepository>(m, "RoleRestriction");
    bind_view<cnf::Role<IdentityTag>, cnf::ConstructorRepository>(m, "RoleIdentity");
    bind_view<cnf::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>, cnf::ConstructorRepository>(m, "BooleanAtomicStateStatic");
    bind_view<cnf::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>, cnf::ConstructorRepository>(m, "BooleanAtomicStateFluent");
    bind_view<cnf::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>, cnf::ConstructorRepository>(m, "BooleanAtomicStateDerived");
    bind_view<cnf::Boolean<NonemptyTag>, cnf::ConstructorRepository>(m, "BooleanNonempty");
    bind_view<cnf::Numerical<CountTag>, cnf::ConstructorRepository>(m, "NumericalCount");
    bind_view<cnf::Numerical<DistanceTag>, cnf::ConstructorRepository>(m, "NumericalDistance");
}

}  // namespace runir::kr::dl
