#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/base/declarations.hpp>
#include <runir/kr/dl/base/repository.hpp>
#include <runir/kr/dl/semantics/base/evaluation_context.hpp>
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

namespace runir::kr::dl::base
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

    bind_view<Constructor<runir::kr::BaseFamilyTag, ConceptTag>, base::ConstructorRepository>(m, "Concept");
    bind_view<Constructor<runir::kr::BaseFamilyTag, RoleTag>, base::ConstructorRepository>(m, "Role");
    bind_view<Constructor<runir::kr::BaseFamilyTag, BooleanTag>, base::ConstructorRepository>(m, "Boolean");
    bind_view<Constructor<runir::kr::BaseFamilyTag, NumericalTag>, base::ConstructorRepository>(m, "Numerical");

    bind_view<Concept<runir::kr::BaseFamilyTag, BotTag>, base::ConstructorRepository>(m, "ConceptBot");
    bind_view<Concept<runir::kr::BaseFamilyTag, TopTag>, base::ConstructorRepository>(m, "ConceptTop");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, base::ConstructorRepository>(m, "ConceptAtomicStateStatic");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, base::ConstructorRepository>(m, "ConceptAtomicStateFluent");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, base::ConstructorRepository>(m, "ConceptAtomicStateDerived");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, base::ConstructorRepository>(m, "ConceptAtomicGoalStatic");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, base::ConstructorRepository>(m, "ConceptAtomicGoalFluent");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, base::ConstructorRepository>(m, "ConceptAtomicGoalDerived");
    bind_view<Concept<runir::kr::BaseFamilyTag, IntersectionTag>, base::ConstructorRepository>(m, "ConceptIntersection");
    bind_view<Concept<runir::kr::BaseFamilyTag, UnionTag>, base::ConstructorRepository>(m, "ConceptUnion");
    bind_view<Concept<runir::kr::BaseFamilyTag, NegationTag>, base::ConstructorRepository>(m, "ConceptNegation");
    bind_view<Concept<runir::kr::BaseFamilyTag, ValueRestrictionTag>, base::ConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, ExistentialQuantificationTag>, base::ConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtLeastNumberRestrictionTag>, base::ConstructorRepository>(m, "ConceptAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtMostNumberRestrictionTag>, base::ConstructorRepository>(m, "ConceptAtMostNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, ExactNumberRestrictionTag>, base::ConstructorRepository>(m, "ConceptExactNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>, base::ConstructorRepository>(m,
                                                                                                                    "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>, base::ConstructorRepository>(m,
                                                                                                                   "ConceptQualifiedAtMostNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, QualifiedExactNumberRestrictionTag>, base::ConstructorRepository>(m, "ConceptQualifiedExactNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, RoleValueMapTag>, base::ConstructorRepository>(m, "ConceptRoleValueMap");
    bind_view<Concept<runir::kr::BaseFamilyTag, AgreementTag>, base::ConstructorRepository>(m, "ConceptAgreement");
    bind_view<Concept<runir::kr::BaseFamilyTag, RoleFillersTag>, base::ConstructorRepository>(m, "ConceptRoleFillers");
    bind_view<Concept<runir::kr::BaseFamilyTag, OneOfTag>, base::ConstructorRepository>(m, "ConceptOneOf");
    bind_view<Concept<runir::kr::BaseFamilyTag, NominalTag>, base::ConstructorRepository>(m, "ConceptNominal");

    bind_view<Role<runir::kr::BaseFamilyTag, UniversalTag>, base::ConstructorRepository>(m, "RoleUniversal");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, base::ConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, base::ConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, base::ConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, base::ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, base::ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, base::ConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<Role<runir::kr::BaseFamilyTag, IntersectionTag>, base::ConstructorRepository>(m, "RoleIntersection");
    bind_view<Role<runir::kr::BaseFamilyTag, UnionTag>, base::ConstructorRepository>(m, "RoleUnion");
    bind_view<Role<runir::kr::BaseFamilyTag, ComplementTag>, base::ConstructorRepository>(m, "RoleComplement");
    bind_view<Role<runir::kr::BaseFamilyTag, InverseTag>, base::ConstructorRepository>(m, "RoleInverse");
    bind_view<Role<runir::kr::BaseFamilyTag, CompositionTag>, base::ConstructorRepository>(m, "RoleComposition");
    bind_view<Role<runir::kr::BaseFamilyTag, TransitiveClosureTag>, base::ConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<Role<runir::kr::BaseFamilyTag, ReflexiveTransitiveClosureTag>, base::ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<Role<runir::kr::BaseFamilyTag, RestrictionTag>, base::ConstructorRepository>(m, "RoleRestriction");
    bind_view<Role<runir::kr::BaseFamilyTag, IdentityTag>, base::ConstructorRepository>(m, "RoleIdentity");

    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, base::ConstructorRepository>(m, "BooleanAtomicStateStatic");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, base::ConstructorRepository>(m, "BooleanAtomicStateFluent");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, base::ConstructorRepository>(m, "BooleanAtomicStateDerived");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, base::ConstructorRepository>(m, "BooleanAtomicGoalStatic");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, base::ConstructorRepository>(m, "BooleanAtomicGoalFluent");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, base::ConstructorRepository>(m, "BooleanAtomicGoalDerived");
    bind_view<Boolean<runir::kr::BaseFamilyTag, NonemptyTag>, base::ConstructorRepository>(m, "BooleanNonempty");

    bind_view<Numerical<runir::kr::BaseFamilyTag, CountTag>, base::ConstructorRepository>(m, "NumericalCount");
    bind_view<Numerical<runir::kr::BaseFamilyTag, DistanceTag>, base::ConstructorRepository>(m, "NumericalDistance");

    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::BaseFamilyTag, ConceptTag, base::ConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::BaseFamilyTag, RoleTag, base::ConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::BaseFamilyTag, BooleanTag, base::ConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::BaseFamilyTag, NumericalTag, base::ConstructorRepository>,
          "constructor"_a);
}

}  // namespace runir::kr::dl::base
