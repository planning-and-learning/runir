#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/views.hpp>
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

void bind_cnf_grammar_views(nb::module_& m)
{
    bind_view<runir::kr::dl::cnf_grammar::Constructor<ConceptTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "Concept");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<RoleTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "Role");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<BooleanTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "Boolean");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<NumericalTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "Numerical");
    bind_view<runir::kr::dl::cnf_grammar::Concept<BotTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "ConceptBot");
    bind_view<runir::kr::dl::cnf_grammar::Concept<TopTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "ConceptTop");
    bind_view<runir::kr::dl::cnf_grammar::Concept<AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "ConceptAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Concept<AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "ConceptAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Concept<AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "ConceptAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Concept<AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "ConceptAtomicGoalStatic");
    bind_view<runir::kr::dl::cnf_grammar::Concept<AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "ConceptAtomicGoalFluent");
    bind_view<runir::kr::dl::cnf_grammar::Concept<AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "ConceptAtomicGoalDerived");
    bind_view<runir::kr::dl::cnf_grammar::Concept<IntersectionTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "ConceptIntersection");
    bind_view<runir::kr::dl::cnf_grammar::Concept<UnionTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "ConceptUnion");
    bind_view<runir::kr::dl::cnf_grammar::Concept<NegationTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "ConceptNegation");
    bind_view<runir::kr::dl::cnf_grammar::Concept<ValueRestrictionTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<ExistentialQuantificationTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "ConceptExistentialQuantification");
    bind_view<runir::kr::dl::cnf_grammar::Concept<RoleValueMapContainmentTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "ConceptRoleValueMapContainment");
    bind_view<runir::kr::dl::cnf_grammar::Concept<RoleValueMapEqualityTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m,
                                                                                                                               "ConceptRoleValueMapEquality");
    bind_view<runir::kr::dl::cnf_grammar::Concept<NominalTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "ConceptNominal");
    bind_view<runir::kr::dl::cnf_grammar::Role<UniversalTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleUniversal");
    bind_view<runir::kr::dl::cnf_grammar::Role<AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "RoleAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Role<AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "RoleAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Role<AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "RoleAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Role<AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "RoleAtomicGoalStatic");
    bind_view<runir::kr::dl::cnf_grammar::Role<AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "RoleAtomicGoalFluent");
    bind_view<runir::kr::dl::cnf_grammar::Role<AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "RoleAtomicGoalDerived");
    bind_view<runir::kr::dl::cnf_grammar::Role<IntersectionTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleIntersection");
    bind_view<runir::kr::dl::cnf_grammar::Role<UnionTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleUnion");
    bind_view<runir::kr::dl::cnf_grammar::Role<ComplementTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleComplement");
    bind_view<runir::kr::dl::cnf_grammar::Role<InverseTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleInverse");
    bind_view<runir::kr::dl::cnf_grammar::Role<CompositionTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleComposition");
    bind_view<runir::kr::dl::cnf_grammar::Role<TransitiveClosureTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<runir::kr::dl::cnf_grammar::Role<ReflexiveTransitiveClosureTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "RoleReflexiveTransitiveClosure");
    bind_view<runir::kr::dl::cnf_grammar::Role<RestrictionTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Role<IdentityTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "RoleIdentity");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "BooleanAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "BooleanAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::cnf_grammar::ConstructorRepository>(
        m,
        "BooleanAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<NonemptyTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "BooleanNonempty");
    bind_view<runir::kr::dl::cnf_grammar::Numerical<CountTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "NumericalCount");
    bind_view<runir::kr::dl::cnf_grammar::Numerical<DistanceTag>, runir::kr::dl::cnf_grammar::ConstructorRepository>(m, "NumericalDistance");
}

}  // namespace runir::kr::dl
