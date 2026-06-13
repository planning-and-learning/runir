#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/ext/declarations.hpp>
#include <runir/kr/dl/ext/repository.hpp>
#include <runir/kr/dl/semantics/ext/evaluation_context.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/dl/semantics/views.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>

namespace runir::kr::dl::ext
{

using namespace nanobind::literals;

namespace
{

template<typename T, typename Repository>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_hash(cls);

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
    if constexpr (requires(const View& view) { view.get_objects(); })
        cls.def("get_objects", &View::get_objects);
    if constexpr (requires(const View& view) { view.get_n(); })
        cls.def("get_n", &View::get_n);
    if constexpr (requires(const View& view) { view.get_role(); })
        cls.def("get_role", &View::get_role);
    if constexpr (requires(const View& view) { view.get_concept(); })
        cls.def("get_concept", &View::get_concept);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, tyr::planning::GroundTag>& context) {
                      runir::kr::dl::semantics::evaluate(view, context);
                  })
        cls.def(
            "evaluate",
            [](const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, tyr::planning::GroundTag>& context)
            { return runir::kr::dl::semantics::evaluate(view, context); },
            "context"_a);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, tyr::planning::LiftedTag>& context) {
                      runir::kr::dl::semantics::evaluate(view, context);
                  })
        cls.def(
            "evaluate",
            [](const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, tyr::planning::LiftedTag>& context)
            { return runir::kr::dl::semantics::evaluate(view, context); },
            "context"_a);
    if constexpr (requires(const View& view) { runir::kr::dl::semantics::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](const View& view) { return runir::kr::dl::semantics::syntactic_complexity(view); });
}

}  // namespace

void bind_semantics_views(nb::module_& m)
{
    // Denotation views are family-independent and registered once by the base semantics module;
    // the evaluate() accessors below still resolve to them.
    bind_view<Constructor<runir::kr::ExtFamilyTag, ConceptTag>, ext::ConstructorRepository>(m, "Concept");
    bind_view<Constructor<runir::kr::ExtFamilyTag, RoleTag>, ext::ConstructorRepository>(m, "Role");
    bind_view<Constructor<runir::kr::ExtFamilyTag, BooleanTag>, ext::ConstructorRepository>(m, "Boolean");
    bind_view<Constructor<runir::kr::ExtFamilyTag, NumericalTag>, ext::ConstructorRepository>(m, "Numerical");

    bind_view<Concept<runir::kr::ExtFamilyTag, BotTag>, ext::ConstructorRepository>(m, "ConceptBot");
    bind_view<Concept<runir::kr::ExtFamilyTag, TopTag>, ext::ConstructorRepository>(m, "ConceptTop");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, ext::ConstructorRepository>(m, "ConceptAtomicStateStatic");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, ext::ConstructorRepository>(m, "ConceptAtomicStateFluent");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, ext::ConstructorRepository>(m, "ConceptAtomicStateDerived");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, ext::ConstructorRepository>(m, "ConceptAtomicGoalStatic");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, ext::ConstructorRepository>(m, "ConceptAtomicGoalFluent");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, ext::ConstructorRepository>(m, "ConceptAtomicGoalDerived");
    bind_view<Concept<runir::kr::ExtFamilyTag, IntersectionTag>, ext::ConstructorRepository>(m, "ConceptIntersection");
    bind_view<Concept<runir::kr::ExtFamilyTag, UnionTag>, ext::ConstructorRepository>(m, "ConceptUnion");
    bind_view<Concept<runir::kr::ExtFamilyTag, NegationTag>, ext::ConstructorRepository>(m, "ConceptNegation");
    bind_view<Concept<runir::kr::ExtFamilyTag, ValueRestrictionTag>, ext::ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, ExistentialQuantificationTag>, ext::ConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtLeastNumberRestrictionTag>, ext::ConstructorRepository>(m, "ConceptAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtMostNumberRestrictionTag>, ext::ConstructorRepository>(m, "ConceptAtMostNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, ExactNumberRestrictionTag>, ext::ConstructorRepository>(m, "ConceptExactNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, QualifiedAtLeastNumberRestrictionTag>, ext::ConstructorRepository>(m, "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, QualifiedAtMostNumberRestrictionTag>, ext::ConstructorRepository>(m, "ConceptQualifiedAtMostNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, QualifiedExactNumberRestrictionTag>, ext::ConstructorRepository>(m, "ConceptQualifiedExactNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, RoleValueMapTag>, ext::ConstructorRepository>(m, "ConceptRoleValueMap");
    bind_view<Concept<runir::kr::ExtFamilyTag, AgreementTag>, ext::ConstructorRepository>(m, "ConceptAgreement");
    bind_view<Concept<runir::kr::ExtFamilyTag, RoleFillersTag>, ext::ConstructorRepository>(m, "ConceptRoleFillers");
    bind_view<Concept<runir::kr::ExtFamilyTag, OneOfTag>, ext::ConstructorRepository>(m, "ConceptOneOf");
    bind_view<Concept<runir::kr::ExtFamilyTag, NominalTag>, ext::ConstructorRepository>(m, "ConceptNominal");
    bind_view<Concept<runir::kr::ExtFamilyTag, RegisterTag>, ext::ConstructorRepository>(m, "ConceptRegister");
    bind_view<Concept<runir::kr::ExtFamilyTag, ArgumentTag<ConceptTag>>, ext::ConstructorRepository>(m, "ConceptArgument");

    bind_view<Role<runir::kr::ExtFamilyTag, UniversalTag>, ext::ConstructorRepository>(m, "RoleUniversal");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, ext::ConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, ext::ConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, ext::ConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, ext::ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, ext::ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, ext::ConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<Role<runir::kr::ExtFamilyTag, IntersectionTag>, ext::ConstructorRepository>(m, "RoleIntersection");
    bind_view<Role<runir::kr::ExtFamilyTag, UnionTag>, ext::ConstructorRepository>(m, "RoleUnion");
    bind_view<Role<runir::kr::ExtFamilyTag, ComplementTag>, ext::ConstructorRepository>(m, "RoleComplement");
    bind_view<Role<runir::kr::ExtFamilyTag, InverseTag>, ext::ConstructorRepository>(m, "RoleInverse");
    bind_view<Role<runir::kr::ExtFamilyTag, CompositionTag>, ext::ConstructorRepository>(m, "RoleComposition");
    bind_view<Role<runir::kr::ExtFamilyTag, TransitiveClosureTag>, ext::ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<Role<runir::kr::ExtFamilyTag, ReflexiveTransitiveClosureTag>, ext::ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<Role<runir::kr::ExtFamilyTag, RestrictionTag>, ext::ConstructorRepository>(m, "RoleRestriction");
    bind_view<Role<runir::kr::ExtFamilyTag, IdentityTag>, ext::ConstructorRepository>(m, "RoleIdentity");
    bind_view<Role<runir::kr::ExtFamilyTag, RegisterTag>, ext::ConstructorRepository>(m, "RoleRegister");
    bind_view<Role<runir::kr::ExtFamilyTag, ArgumentTag<RoleTag>>, ext::ConstructorRepository>(m, "RoleArgument");

    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, ext::ConstructorRepository>(m, "BooleanAtomicStateStatic");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, ext::ConstructorRepository>(m, "BooleanAtomicStateFluent");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, ext::ConstructorRepository>(m, "BooleanAtomicStateDerived");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, ext::ConstructorRepository>(m, "BooleanAtomicGoalStatic");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, ext::ConstructorRepository>(m, "BooleanAtomicGoalFluent");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, ext::ConstructorRepository>(m, "BooleanAtomicGoalDerived");
    bind_view<Boolean<runir::kr::ExtFamilyTag, NonemptyTag>, ext::ConstructorRepository>(m, "BooleanNonempty");
    bind_view<Boolean<runir::kr::ExtFamilyTag, ArgumentTag<BooleanTag>>, ext::ConstructorRepository>(m, "BooleanArgument");

    bind_view<Numerical<runir::kr::ExtFamilyTag, CountTag>, ext::ConstructorRepository>(m, "NumericalCount");
    bind_view<Numerical<runir::kr::ExtFamilyTag, DistanceTag>, ext::ConstructorRepository>(m, "NumericalDistance");
    bind_view<Numerical<runir::kr::ExtFamilyTag, ArgumentTag<NumericalTag>>, ext::ConstructorRepository>(m, "NumericalArgument");

    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::ExtFamilyTag, ConceptTag, ext::ConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::ExtFamilyTag, RoleTag, ext::ConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::ExtFamilyTag, BooleanTag, ext::ConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::ExtFamilyTag, NumericalTag, ext::ConstructorRepository>,
          "constructor"_a);
}

}  // namespace runir::kr::dl::ext
