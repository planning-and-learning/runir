#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/dl/semantics/views.hpp>
#include <tyr/common/python/bindings.hpp>
#include <tyr/common/python/type_casters.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>

namespace runir::kr::dl
{

using namespace nanobind::literals;

namespace
{

template<CategoryTag Category>
void bind_denotation_view(nb::module_& m, const std::string& name)
{
    using View = tyr::View<tyr::Index<runir::kr::dl::semantics::Denotation<Category>>, runir::kr::dl::semantics::DenotationRepository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    tyr::add_print(cls);
    tyr::add_hash(cls);

    if constexpr (std::same_as<Category, BooleanTag> || std::same_as<Category, NumericalTag>)
        cls.def("get", [](const View& view) { return view.get(); });
}

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
    if constexpr (requires(const View& view) { view.get_objects(); })
        cls.def("get_objects", &View::get_objects);
    if constexpr (requires(const View& view) { view.get_n(); })
        cls.def("get_n", &View::get_n);
    if constexpr (requires(const View& view) { view.get_role(); })
        cls.def("get_role", &View::get_role);
    if constexpr (requires(const View& view) { view.get_concept(); })
        cls.def("get_concept", &View::get_concept);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>& context) {
                      runir::kr::dl::semantics::evaluate(view, context);
                  })
        cls.def(
            "evaluate",
            [](const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>& context)
            { return runir::kr::dl::semantics::evaluate(view, context); },
            "context"_a);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>& context) {
                      runir::kr::dl::semantics::evaluate(view, context);
                  })
        cls.def(
            "evaluate",
            [](const View& view, runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>& context)
            { return runir::kr::dl::semantics::evaluate(view, context); },
            "context"_a);
    if constexpr (requires(const View& view) { runir::kr::dl::semantics::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](const View& view) { return runir::kr::dl::semantics::syntactic_complexity(view); });
}

}  // namespace

void bind_semantics_views(nb::module_& m)
{
    bind_denotation_view<BooleanTag>(m, "BooleanDenotation");
    bind_denotation_view<NumericalTag>(m, "NumericalDenotation");
    bind_denotation_view<ConceptTag>(m, "ConceptDenotation");
    bind_denotation_view<RoleTag>(m, "RoleDenotation");

    bind_view<Constructor<BaseFamilyTag, ConceptTag>, ConstructorRepository>(m, "Concept");
    bind_view<Constructor<BaseFamilyTag, RoleTag>, ConstructorRepository>(m, "Role");
    bind_view<Constructor<BaseFamilyTag, BooleanTag>, ConstructorRepository>(m, "Boolean");
    bind_view<Constructor<BaseFamilyTag, NumericalTag>, ConstructorRepository>(m, "Numerical");

    bind_view<Concept<BaseFamilyTag, BotTag>, ConstructorRepository>(m, "ConceptBot");
    bind_view<Concept<BaseFamilyTag, TopTag>, ConstructorRepository>(m, "ConceptTop");
    bind_view<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "ConceptAtomicStateStatic");
    bind_view<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "ConceptAtomicStateFluent");
    bind_view<Concept<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "ConceptAtomicStateDerived");
    bind_view<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "ConceptAtomicGoalStatic");
    bind_view<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "ConceptAtomicGoalFluent");
    bind_view<Concept<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "ConceptAtomicGoalDerived");
    bind_view<Concept<BaseFamilyTag, IntersectionTag>, ConstructorRepository>(m, "ConceptIntersection");
    bind_view<Concept<BaseFamilyTag, UnionTag>, ConstructorRepository>(m, "ConceptUnion");
    bind_view<Concept<BaseFamilyTag, NegationTag>, ConstructorRepository>(m, "ConceptNegation");
    bind_view<Concept<BaseFamilyTag, ValueRestrictionTag>, ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<Concept<BaseFamilyTag, ExistentialQuantificationTag>, ConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<Concept<BaseFamilyTag, AtLeastNumberRestrictionTag>, ConstructorRepository>(m, "ConceptAtLeastNumberRestriction");
    bind_view<Concept<BaseFamilyTag, AtMostNumberRestrictionTag>, ConstructorRepository>(m, "ConceptAtMostNumberRestriction");
    bind_view<Concept<BaseFamilyTag, ExactNumberRestrictionTag>, ConstructorRepository>(m, "ConceptExactNumberRestriction");
    bind_view<Concept<BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>, ConstructorRepository>(m, "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<Concept<BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>, ConstructorRepository>(m, "ConceptQualifiedAtMostNumberRestriction");
    bind_view<Concept<BaseFamilyTag, QualifiedExactNumberRestrictionTag>, ConstructorRepository>(m, "ConceptQualifiedExactNumberRestriction");
    bind_view<Concept<BaseFamilyTag, RoleValueMapTag>, ConstructorRepository>(m, "ConceptRoleValueMap");
    bind_view<Concept<BaseFamilyTag, AgreementTag>, ConstructorRepository>(m, "ConceptAgreement");
    bind_view<Concept<BaseFamilyTag, RoleFillersTag>, ConstructorRepository>(m, "ConceptRoleFillers");
    bind_view<Concept<BaseFamilyTag, OneOfTag>, ConstructorRepository>(m, "ConceptOneOf");
    bind_view<Concept<BaseFamilyTag, NominalTag>, ConstructorRepository>(m, "ConceptNominal");

    bind_view<Role<BaseFamilyTag, UniversalTag>, ConstructorRepository>(m, "RoleUniversal");
    bind_view<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<Role<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<Role<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<Role<BaseFamilyTag, IntersectionTag>, ConstructorRepository>(m, "RoleIntersection");
    bind_view<Role<BaseFamilyTag, UnionTag>, ConstructorRepository>(m, "RoleUnion");
    bind_view<Role<BaseFamilyTag, ComplementTag>, ConstructorRepository>(m, "RoleComplement");
    bind_view<Role<BaseFamilyTag, InverseTag>, ConstructorRepository>(m, "RoleInverse");
    bind_view<Role<BaseFamilyTag, CompositionTag>, ConstructorRepository>(m, "RoleComposition");
    bind_view<Role<BaseFamilyTag, TransitiveClosureTag>, ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<Role<BaseFamilyTag, ReflexiveTransitiveClosureTag>, ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<Role<BaseFamilyTag, RestrictionTag>, ConstructorRepository>(m, "RoleRestriction");
    bind_view<Role<BaseFamilyTag, IdentityTag>, ConstructorRepository>(m, "RoleIdentity");

    bind_view<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "BooleanAtomicStateStatic");
    bind_view<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "BooleanAtomicStateFluent");
    bind_view<Boolean<BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "BooleanAtomicStateDerived");
    bind_view<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, ConstructorRepository>(m, "BooleanAtomicGoalStatic");
    bind_view<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, ConstructorRepository>(m, "BooleanAtomicGoalFluent");
    bind_view<Boolean<BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, ConstructorRepository>(m, "BooleanAtomicGoalDerived");
    bind_view<Boolean<BaseFamilyTag, NonemptyTag>, ConstructorRepository>(m, "BooleanNonempty");

    bind_view<Numerical<BaseFamilyTag, CountTag>, ConstructorRepository>(m, "NumericalCount");
    bind_view<Numerical<BaseFamilyTag, DistanceTag>, ConstructorRepository>(m, "NumericalDistance");

    m.def("syntactic_complexity", &runir::kr::dl::semantics::syntactic_complexity<BaseFamilyTag, ConceptTag, ConstructorRepository>, "constructor"_a);
    m.def("syntactic_complexity", &runir::kr::dl::semantics::syntactic_complexity<BaseFamilyTag, RoleTag, ConstructorRepository>, "constructor"_a);
    m.def("syntactic_complexity", &runir::kr::dl::semantics::syntactic_complexity<BaseFamilyTag, BooleanTag, ConstructorRepository>, "constructor"_a);
    m.def("syntactic_complexity", &runir::kr::dl::semantics::syntactic_complexity<BaseFamilyTag, NumericalTag, ConstructorRepository>, "constructor"_a);
}

}  // namespace runir::kr::dl
