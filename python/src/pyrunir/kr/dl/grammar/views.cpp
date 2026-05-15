#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <runir/kr/dl/grammar/views.hpp>
#include <tyr/common/python/bindings.hpp>
#include <tyr/common/python/type_casters.hpp>

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
    if constexpr (requires(const View& view) { view.get_name(); })
        cls.def("get_name", &View::get_name);
    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
}

}  // namespace

void bind_grammar_views(nb::module_& m)
{
    bind_view<runir::kr::dl::grammar::Constructor<ConceptTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "Concept");
    bind_view<runir::kr::dl::grammar::Constructor<RoleTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "Role");
    bind_view<runir::kr::dl::grammar::Constructor<BooleanTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "Boolean");
    bind_view<runir::kr::dl::grammar::Constructor<NumericalTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "Numerical");
    bind_view<runir::kr::dl::grammar::Concept<BotTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptBot");
    bind_view<runir::kr::dl::grammar::Concept<TopTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptTop");
    bind_view<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "ConceptAtomicStateStatic");
    bind_view<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "ConceptAtomicStateFluent");
    bind_view<runir::kr::dl::grammar::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "ConceptAtomicStateDerived");
    bind_view<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "ConceptAtomicGoalStatic");
    bind_view<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "ConceptAtomicGoalFluent");
    bind_view<runir::kr::dl::grammar::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "ConceptAtomicGoalDerived");
    bind_view<runir::kr::dl::grammar::Concept<IntersectionTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptIntersection");
    bind_view<runir::kr::dl::grammar::Concept<UnionTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptUnion");
    bind_view<runir::kr::dl::grammar::Concept<NegationTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptNegation");
    bind_view<runir::kr::dl::grammar::Concept<ValueRestrictionTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<runir::kr::dl::grammar::Concept<ExistentialQuantificationTag>, runir::kr::dl::grammar::ConstructorRepository>(m,
                                                                                                                            "ConceptExistentialQuantification");
    bind_view<runir::kr::dl::grammar::Concept<RoleValueMapContainmentTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptRoleValueMapContainment");
    bind_view<runir::kr::dl::grammar::Concept<RoleValueMapEqualityTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptRoleValueMapEquality");
    bind_view<runir::kr::dl::grammar::Concept<NominalTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "ConceptNominal");
    bind_view<runir::kr::dl::grammar::Role<UniversalTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleUniversal");
    bind_view<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::grammar::ConstructorRepository>(m,
                                                                                                                                      "RoleAtomicStateStatic");
    bind_view<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::grammar::ConstructorRepository>(m,
                                                                                                                                      "RoleAtomicStateFluent");
    bind_view<runir::kr::dl::grammar::Role<AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "RoleAtomicStateDerived");
    bind_view<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<runir::kr::dl::grammar::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::grammar::ConstructorRepository>(m,
                                                                                                                                      "RoleAtomicGoalDerived");
    bind_view<runir::kr::dl::grammar::Role<IntersectionTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleIntersection");
    bind_view<runir::kr::dl::grammar::Role<UnionTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleUnion");
    bind_view<runir::kr::dl::grammar::Role<ComplementTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleComplement");
    bind_view<runir::kr::dl::grammar::Role<InverseTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleInverse");
    bind_view<runir::kr::dl::grammar::Role<CompositionTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleComposition");
    bind_view<runir::kr::dl::grammar::Role<TransitiveClosureTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<runir::kr::dl::grammar::Role<ReflexiveTransitiveClosureTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<runir::kr::dl::grammar::Role<RestrictionTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleRestriction");
    bind_view<runir::kr::dl::grammar::Role<IdentityTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "RoleIdentity");
    bind_view<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "BooleanAtomicStateStatic");
    bind_view<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "BooleanAtomicStateFluent");
    bind_view<runir::kr::dl::grammar::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::grammar::ConstructorRepository>(
        m,
        "BooleanAtomicStateDerived");
    bind_view<runir::kr::dl::grammar::Boolean<NonemptyTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "BooleanNonempty");
    bind_view<runir::kr::dl::grammar::Numerical<CountTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "NumericalCount");
    bind_view<runir::kr::dl::grammar::Numerical<DistanceTag>, runir::kr::dl::grammar::ConstructorRepository>(m, "NumericalDistance");
}

}  // namespace runir::kr::dl
