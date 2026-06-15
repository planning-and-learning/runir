#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/uns/declarations.hpp>
#include <runir/kr/dl/uns/repository.hpp>
#include <runir/kr/dl/semantics/uns/evaluation.hpp>
#include <runir/kr/dl/semantics/uns/evaluation_context.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/dl/semantics/views.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>

namespace runir::kr::dl::uns
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
    if constexpr (requires(const View& view) { view.get_arg(); }) cls.def("get_arg", &View::get_arg);
    if constexpr (requires(const View& view) { view.get_lhs(); }) cls.def("get_lhs", &View::get_lhs);
    if constexpr (requires(const View& view) { view.get_mid(); }) cls.def("get_mid", &View::get_mid);
    if constexpr (requires(const View& view) { view.get_rhs(); }) cls.def("get_rhs", &View::get_rhs);
    if constexpr (requires(const View& view) { view.get_value(); }) cls.def("get_value", &View::get_value);
    if constexpr (requires(const View& view) { view.get_predicate(); }) cls.def("get_predicate", &View::get_predicate);
    if constexpr (requires(const View& view) { view.get_polarity(); }) cls.def("get_polarity", &View::get_polarity);
    if constexpr (requires(const View& view) { view.get_object(); }) cls.def("get_object", &View::get_object);
    if constexpr (requires(const View& view) { view.get_objects(); }) cls.def("get_objects", &View::get_objects);
    if constexpr (requires(const View& view) { view.get_n(); }) cls.def("get_n", &View::get_n);
    if constexpr (requires(const View& view) { view.get_role(); }) cls.def("get_role", &View::get_role);
    if constexpr (requires(const View& view) { view.get_concept(); }) cls.def("get_concept", &View::get_concept);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, tyr::planning::GroundTag>& context) {
                      runir::kr::dl::semantics::evaluate(view, context);
                  })
        cls.def("evaluate",
                [](const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, tyr::planning::GroundTag>& context)
                { return runir::kr::dl::semantics::evaluate(view, context); },
                "context"_a);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, tyr::planning::LiftedTag>& context) {
                      runir::kr::dl::semantics::evaluate(view, context);
                  })
        cls.def("evaluate",
                [](const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, tyr::planning::LiftedTag>& context)
                { return runir::kr::dl::semantics::evaluate(view, context); },
                "context"_a);
    if constexpr (requires(const View& view) { runir::kr::dl::semantics::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](const View& view) { return runir::kr::dl::semantics::syntactic_complexity(view); });
}

}  // namespace

void bind_semantics_views(nb::module_& m)
{
    bind_view<Constructor<runir::kr::UnsFamilyTag, ConceptTag>, uns::ConstructorRepository>(m, "Concept");
    bind_view<Constructor<runir::kr::UnsFamilyTag, RoleTag>, uns::ConstructorRepository>(m, "Role");
    bind_view<Constructor<runir::kr::UnsFamilyTag, BooleanTag>, uns::ConstructorRepository>(m, "Boolean");
    bind_view<Constructor<runir::kr::UnsFamilyTag, NumericalTag>, uns::ConstructorRepository>(m, "Numerical");

    bind_view<Concept<runir::kr::UnsFamilyTag, BotTag>, uns::ConstructorRepository>(m, "ConceptBot");
    bind_view<Concept<runir::kr::UnsFamilyTag, TopTag>, uns::ConstructorRepository>(m, "ConceptTop");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, uns::ConstructorRepository>(m, "ConceptAtomicStateStatic");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, uns::ConstructorRepository>(m, "ConceptAtomicStateFluent");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, uns::ConstructorRepository>(m, "ConceptAtomicStateDerived");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, uns::ConstructorRepository>(m, "ConceptAtomicGoalStatic");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, uns::ConstructorRepository>(m, "ConceptAtomicGoalFluent");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, uns::ConstructorRepository>(m, "ConceptAtomicGoalDerived");
    bind_view<Concept<runir::kr::UnsFamilyTag, IntersectionTag>, uns::ConstructorRepository>(m, "ConceptIntersection");
    bind_view<Concept<runir::kr::UnsFamilyTag, UnionTag>, uns::ConstructorRepository>(m, "ConceptUnion");
    bind_view<Concept<runir::kr::UnsFamilyTag, NegationTag>, uns::ConstructorRepository>(m, "ConceptNegation");
    bind_view<Concept<runir::kr::UnsFamilyTag, ValueRestrictionTag>, uns::ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, ExistentialQuantificationTag>, uns::ConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtLeastNumberRestrictionTag>, uns::ConstructorRepository>(m, "ConceptAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtMostNumberRestrictionTag>, uns::ConstructorRepository>(m, "ConceptAtMostNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, ExactNumberRestrictionTag>, uns::ConstructorRepository>(m, "ConceptExactNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, QualifiedAtLeastNumberRestrictionTag>, uns::ConstructorRepository>(m, "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, QualifiedAtMostNumberRestrictionTag>, uns::ConstructorRepository>(m, "ConceptQualifiedAtMostNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, QualifiedExactNumberRestrictionTag>, uns::ConstructorRepository>(m, "ConceptQualifiedExactNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, RoleValueMapTag>, uns::ConstructorRepository>(m, "ConceptRoleValueMap");
    bind_view<Concept<runir::kr::UnsFamilyTag, AgreementTag>, uns::ConstructorRepository>(m, "ConceptAgreement");
    bind_view<Concept<runir::kr::UnsFamilyTag, RoleFillersTag>, uns::ConstructorRepository>(m, "ConceptRoleFillers");
    bind_view<Concept<runir::kr::UnsFamilyTag, OneOfTag>, uns::ConstructorRepository>(m, "ConceptOneOf");
    bind_view<Concept<runir::kr::UnsFamilyTag, NominalTag>, uns::ConstructorRepository>(m, "ConceptNominal");

    bind_view<Role<runir::kr::UnsFamilyTag, UniversalTag>, uns::ConstructorRepository>(m, "RoleUniversal");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, uns::ConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, uns::ConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, uns::ConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, uns::ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, uns::ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, uns::ConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<Role<runir::kr::UnsFamilyTag, IntersectionTag>, uns::ConstructorRepository>(m, "RoleIntersection");
    bind_view<Role<runir::kr::UnsFamilyTag, UnionTag>, uns::ConstructorRepository>(m, "RoleUnion");
    bind_view<Role<runir::kr::UnsFamilyTag, ComplementTag>, uns::ConstructorRepository>(m, "RoleComplement");
    bind_view<Role<runir::kr::UnsFamilyTag, InverseTag>, uns::ConstructorRepository>(m, "RoleInverse");
    bind_view<Role<runir::kr::UnsFamilyTag, CompositionTag>, uns::ConstructorRepository>(m, "RoleComposition");
    bind_view<Role<runir::kr::UnsFamilyTag, TransitiveClosureTag>, uns::ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<Role<runir::kr::UnsFamilyTag, ReflexiveTransitiveClosureTag>, uns::ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<Role<runir::kr::UnsFamilyTag, RestrictionTag>, uns::ConstructorRepository>(m, "RoleRestriction");
    bind_view<Role<runir::kr::UnsFamilyTag, IdentityTag>, uns::ConstructorRepository>(m, "RoleIdentity");

    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, uns::ConstructorRepository>(m, "BooleanAtomicStateStatic");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, uns::ConstructorRepository>(m, "BooleanAtomicStateFluent");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, uns::ConstructorRepository>(m, "BooleanAtomicStateDerived");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, uns::ConstructorRepository>(m, "BooleanAtomicGoalStatic");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, uns::ConstructorRepository>(m, "BooleanAtomicGoalFluent");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, uns::ConstructorRepository>(m, "BooleanAtomicGoalDerived");
    bind_view<Boolean<runir::kr::UnsFamilyTag, NonemptyTag>, uns::ConstructorRepository>(m, "BooleanNonempty");
    bind_view<Boolean<runir::kr::UnsFamilyTag, EqTag<BooleanTag>>, uns::ConstructorRepository>(m, "BooleanEq");
    bind_view<Boolean<runir::kr::UnsFamilyTag, NeqTag<BooleanTag>>, uns::ConstructorRepository>(m, "BooleanNeq");
    bind_view<Boolean<runir::kr::UnsFamilyTag, LtTag<BooleanTag>>, uns::ConstructorRepository>(m, "BooleanLt");
    bind_view<Boolean<runir::kr::UnsFamilyTag, LeTag<BooleanTag>>, uns::ConstructorRepository>(m, "BooleanLe");
    bind_view<Boolean<runir::kr::UnsFamilyTag, GtTag<BooleanTag>>, uns::ConstructorRepository>(m, "BooleanGt");
    bind_view<Boolean<runir::kr::UnsFamilyTag, GeTag<BooleanTag>>, uns::ConstructorRepository>(m, "BooleanGe");
    bind_view<Boolean<runir::kr::UnsFamilyTag, EqTag<NumericalTag>>, uns::ConstructorRepository>(m, "NumericalEq");
    bind_view<Boolean<runir::kr::UnsFamilyTag, NeqTag<NumericalTag>>, uns::ConstructorRepository>(m, "NumericalNeq");
    bind_view<Boolean<runir::kr::UnsFamilyTag, LtTag<NumericalTag>>, uns::ConstructorRepository>(m, "NumericalLt");
    bind_view<Boolean<runir::kr::UnsFamilyTag, LeTag<NumericalTag>>, uns::ConstructorRepository>(m, "NumericalLe");
    bind_view<Boolean<runir::kr::UnsFamilyTag, GtTag<NumericalTag>>, uns::ConstructorRepository>(m, "NumericalGt");
    bind_view<Boolean<runir::kr::UnsFamilyTag, GeTag<NumericalTag>>, uns::ConstructorRepository>(m, "NumericalGe");
    bind_view<Boolean<runir::kr::UnsFamilyTag, BooleanConstantTag>, uns::ConstructorRepository>(m, "BooleanConstant");

    bind_view<Numerical<runir::kr::UnsFamilyTag, CountTag>, uns::ConstructorRepository>(m, "NumericalCount");
    bind_view<Numerical<runir::kr::UnsFamilyTag, DistanceTag>, uns::ConstructorRepository>(m, "NumericalDistance");
    bind_view<Numerical<runir::kr::UnsFamilyTag, NumericalConstantTag>, uns::ConstructorRepository>(m, "NumericalConstant");

    m.def("syntactic_complexity", &runir::kr::dl::semantics::syntactic_complexity<runir::kr::UnsFamilyTag, ConceptTag, uns::ConstructorRepository>, "constructor"_a);
    m.def("syntactic_complexity", &runir::kr::dl::semantics::syntactic_complexity<runir::kr::UnsFamilyTag, RoleTag, uns::ConstructorRepository>, "constructor"_a);
    m.def("syntactic_complexity", &runir::kr::dl::semantics::syntactic_complexity<runir::kr::UnsFamilyTag, BooleanTag, uns::ConstructorRepository>, "constructor"_a);
    m.def("syntactic_complexity", &runir::kr::dl::semantics::syntactic_complexity<runir::kr::UnsFamilyTag, NumericalTag, uns::ConstructorRepository>, "constructor"_a);
}

}  // namespace runir::kr::dl::uns
