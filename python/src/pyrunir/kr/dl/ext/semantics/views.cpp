#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/declarations.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/ext/evaluation_context.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/dl/semantics/views.hpp>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::ext
{

using namespace nanobind::literals;

namespace
{

template<typename T, typename Repository>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    using GroundContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, tyr::planning::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, tyr::planning::LiftedTag>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_hash(cls);

    if constexpr (requires(const View& view) { view.get_arg(); })
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_lhs(); })
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_mid(); })
        cls.def("get_mid", &View::get_mid, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_rhs(); })
        cls.def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_predicate(); })
        cls.def("get_predicate", &View::get_predicate, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_polarity(); })
        cls.def("get_polarity", &View::get_polarity);
    if constexpr (requires(const View& view) { view.get_object(); })
        cls.def("get_object", &View::get_object, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_objects(); })
        cls.def("get_objects", &View::get_objects, nb::rv_policy::reference_internal);
    if constexpr (requires(const View& view) { view.get_n(); })
        cls.def("get_n", &View::get_n);
    if constexpr (requires(const View& view) { view.get_role(); })
        cls.def("get_role", &View::get_role, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_concept(); })
        cls.def("get_concept", &View::get_concept, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view, GroundContext& context) { runir::kr::dl::semantics::evaluate(view, context); })
        cls.def("evaluate", nb::overload_cast<View, GroundContext&>(&runir::kr::dl::semantics::evaluate), "context"_a);
    if constexpr (requires(const View& view, LiftedContext& context) { runir::kr::dl::semantics::evaluate(view, context); })
        cls.def("evaluate", nb::overload_cast<View, LiftedContext&>(&runir::kr::dl::semantics::evaluate), "context"_a);
    if constexpr (requires(const View& view) { runir::kr::dl::semantics::syntactic_complexity(view); })
        cls.def("syntactic_complexity", nb::overload_cast<View>(&runir::kr::dl::semantics::syntactic_complexity));
}

}  // namespace

void bind_semantics_views(nb::module_& m)
{
    // Denotation views are family-independent and registered once by the base semantics module;
    // the evaluate() accessors below still resolve to them.
    bind_view<Constructor<runir::kr::ExtFamilyTag, ConceptTag>, runir::kr::dl::ExtConstructorRepository>(m, "Concept");
    bind_view<Constructor<runir::kr::ExtFamilyTag, RoleTag>, runir::kr::dl::ExtConstructorRepository>(m, "Role");
    bind_view<Constructor<runir::kr::ExtFamilyTag, BooleanTag>, runir::kr::dl::ExtConstructorRepository>(m, "Boolean");
    bind_view<Constructor<runir::kr::ExtFamilyTag, NumericalTag>, runir::kr::dl::ExtConstructorRepository>(m, "Numerical");

    bind_view<Concept<runir::kr::ExtFamilyTag, BotTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptBot");
    bind_view<Concept<runir::kr::ExtFamilyTag, TopTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptTop");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                    "ConceptAtomicStateStatic");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                    "ConceptAtomicStateFluent");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::ExtConstructorRepository>(
        m,
        "ConceptAtomicStateDerived");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                   "ConceptAtomicGoalStatic");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                   "ConceptAtomicGoalFluent");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                    "ConceptAtomicGoalDerived");
    bind_view<Concept<runir::kr::ExtFamilyTag, IntersectionTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptIntersection");
    bind_view<Concept<runir::kr::ExtFamilyTag, UnionTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptUnion");
    bind_view<Concept<runir::kr::ExtFamilyTag, NegationTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptNegation");
    bind_view<Concept<runir::kr::ExtFamilyTag, ValueRestrictionTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, ExistentialQuantificationTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtLeastNumberRestrictionTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, AtMostNumberRestrictionTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptAtMostNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, ExactNumberRestrictionTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptExactNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, QualifiedAtLeastNumberRestrictionTag>, runir::kr::dl::ExtConstructorRepository>(
        m,
        "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, QualifiedAtMostNumberRestrictionTag>, runir::kr::dl::ExtConstructorRepository>(
        m,
        "ConceptQualifiedAtMostNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, QualifiedExactNumberRestrictionTag>, runir::kr::dl::ExtConstructorRepository>(
        m,
        "ConceptQualifiedExactNumberRestriction");
    bind_view<Concept<runir::kr::ExtFamilyTag, RoleValueMapTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptRoleValueMap");
    bind_view<Concept<runir::kr::ExtFamilyTag, AgreementTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptAgreement");
    bind_view<Concept<runir::kr::ExtFamilyTag, RoleFillersTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptRoleFillers");
    bind_view<Concept<runir::kr::ExtFamilyTag, OneOfTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptOneOf");
    bind_view<Concept<runir::kr::ExtFamilyTag, NominalTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptNominal");
    bind_view<Concept<runir::kr::ExtFamilyTag, RegisterTag>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptRegister");
    bind_view<Concept<runir::kr::ExtFamilyTag, ArgumentTag<ConceptTag>>, runir::kr::dl::ExtConstructorRepository>(m, "ConceptArgument");

    bind_view<Role<runir::kr::ExtFamilyTag, UniversalTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleUniversal");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::ExtConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::ExtConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::ExtConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::ExtConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::ExtConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<Role<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::ExtConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<Role<runir::kr::ExtFamilyTag, IntersectionTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleIntersection");
    bind_view<Role<runir::kr::ExtFamilyTag, UnionTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleUnion");
    bind_view<Role<runir::kr::ExtFamilyTag, ComplementTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleComplement");
    bind_view<Role<runir::kr::ExtFamilyTag, InverseTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleInverse");
    bind_view<Role<runir::kr::ExtFamilyTag, CompositionTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleComposition");
    bind_view<Role<runir::kr::ExtFamilyTag, TransitiveClosureTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<Role<runir::kr::ExtFamilyTag, ReflexiveTransitiveClosureTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<Role<runir::kr::ExtFamilyTag, RestrictionTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleRestriction");
    bind_view<Role<runir::kr::ExtFamilyTag, IdentityTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleIdentity");
    bind_view<Role<runir::kr::ExtFamilyTag, RegisterTag>, runir::kr::dl::ExtConstructorRepository>(m, "RoleRegister");
    bind_view<Role<runir::kr::ExtFamilyTag, ArgumentTag<RoleTag>>, runir::kr::dl::ExtConstructorRepository>(m, "RoleArgument");

    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                    "BooleanAtomicStateStatic");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                    "BooleanAtomicStateFluent");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::ExtConstructorRepository>(
        m,
        "BooleanAtomicStateDerived");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                   "BooleanAtomicGoalStatic");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                   "BooleanAtomicGoalFluent");
    bind_view<Boolean<runir::kr::ExtFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::ExtConstructorRepository>(m,
                                                                                                                                    "BooleanAtomicGoalDerived");
    bind_view<Boolean<runir::kr::ExtFamilyTag, NonemptyTag>, runir::kr::dl::ExtConstructorRepository>(m, "BooleanNonempty");
    bind_view<Boolean<runir::kr::ExtFamilyTag, ArgumentTag<BooleanTag>>, runir::kr::dl::ExtConstructorRepository>(m, "BooleanArgument");

    bind_view<Numerical<runir::kr::ExtFamilyTag, CountTag>, runir::kr::dl::ExtConstructorRepository>(m, "NumericalCount");
    bind_view<Numerical<runir::kr::ExtFamilyTag, DistanceTag>, runir::kr::dl::ExtConstructorRepository>(m, "NumericalDistance");
    bind_view<Numerical<runir::kr::ExtFamilyTag, ArgumentTag<NumericalTag>>, runir::kr::dl::ExtConstructorRepository>(m, "NumericalArgument");

    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::ExtFamilyTag, ConceptTag, runir::kr::dl::ExtConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::ExtFamilyTag, RoleTag, runir::kr::dl::ExtConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::ExtFamilyTag, BooleanTag, runir::kr::dl::ExtConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::ExtFamilyTag, NumericalTag, runir::kr::dl::ExtConstructorRepository>,
          "constructor"_a);
}

}  // namespace runir::kr::dl::ext
