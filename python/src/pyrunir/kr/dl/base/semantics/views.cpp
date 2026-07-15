#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/declarations.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/base/evaluation_context.hpp>
#include <runir/kr/dl/semantics/evaluation.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/dl/semantics/views.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{

using namespace nanobind::literals;

namespace
{

template<CategoryTag Category>
void bind_denotation_view(nb::module_& m, const std::string& name)
{
    using View = ygg::View<ygg::Index<runir::kr::dl::semantics::Denotation<Category>>, runir::kr::dl::semantics::DenotationRepository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_hash(cls);

    if constexpr (std::same_as<Category, BooleanTag> || std::same_as<Category, NumericalTag>)
        cls.def("get", [](View view) { return view.get(); });
}

template<typename T, typename Repository>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    using GroundContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>;

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
        cls.def("get_objects", &View::get_objects);
    if constexpr (requires(const View& view) { view.get_n(); })
        cls.def("get_n", &View::get_n);
    if constexpr (requires(const View& view) { view.get_role(); })
        cls.def("get_role", &View::get_role, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_concept(); })
        cls.def("get_concept", &View::get_concept, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view, GroundContext& context) { runir::kr::dl::semantics::evaluate(view, context); })
        cls.def("evaluate", [](View view, GroundContext& context) { return runir::kr::dl::semantics::evaluate(view, context); }, "context"_a);
    if constexpr (requires(const View& view, LiftedContext& context) { runir::kr::dl::semantics::evaluate(view, context); })
        cls.def("evaluate", [](View view, LiftedContext& context) { return runir::kr::dl::semantics::evaluate(view, context); }, "context"_a);
    if constexpr (requires(const View& view) { runir::kr::dl::semantics::syntactic_complexity(view); })
        cls.def("syntactic_complexity", [](View view) { return runir::kr::dl::semantics::syntactic_complexity(view); });
}

}  // namespace

void bind_semantics_views(nb::module_& m)
{
    bind_denotation_view<BooleanTag>(m, "BooleanDenotation");
    bind_denotation_view<NumericalTag>(m, "NumericalDenotation");
    bind_denotation_view<ConceptTag>(m, "ConceptDenotation");
    bind_denotation_view<RoleTag>(m, "RoleDenotation");

    bind_view<Constructor<runir::kr::BaseFamilyTag, ConceptTag>, runir::kr::dl::BaseConstructorRepository>(m, "Concept");
    bind_view<Constructor<runir::kr::BaseFamilyTag, RoleTag>, runir::kr::dl::BaseConstructorRepository>(m, "Role");
    bind_view<Constructor<runir::kr::BaseFamilyTag, BooleanTag>, runir::kr::dl::BaseConstructorRepository>(m, "Boolean");
    bind_view<Constructor<runir::kr::BaseFamilyTag, NumericalTag>, runir::kr::dl::BaseConstructorRepository>(m, "Numerical");

    bind_view<Concept<runir::kr::BaseFamilyTag, BotTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptBot");
    bind_view<Concept<runir::kr::BaseFamilyTag, TopTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptTop");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "ConceptAtomicStateStatic");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "ConceptAtomicStateFluent");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "ConceptAtomicStateDerived");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::BaseConstructorRepository>(m,
                                                                                                                                     "ConceptAtomicGoalStatic");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::BaseConstructorRepository>(m,
                                                                                                                                     "ConceptAtomicGoalFluent");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "ConceptAtomicGoalDerived");
    bind_view<Concept<runir::kr::BaseFamilyTag, IntersectionTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptIntersection");
    bind_view<Concept<runir::kr::BaseFamilyTag, UnionTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptUnion");
    bind_view<Concept<runir::kr::BaseFamilyTag, NegationTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptNegation");
    bind_view<Concept<runir::kr::BaseFamilyTag, ValueRestrictionTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, ExistentialQuantificationTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtLeastNumberRestrictionTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, AtMostNumberRestrictionTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptAtMostNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, ExactNumberRestrictionTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptExactNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "ConceptQualifiedAtMostNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, QualifiedExactNumberRestrictionTag>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "ConceptQualifiedExactNumberRestriction");
    bind_view<Concept<runir::kr::BaseFamilyTag, RoleValueMapTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptRoleValueMap");
    bind_view<Concept<runir::kr::BaseFamilyTag, AgreementTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptAgreement");
    bind_view<Concept<runir::kr::BaseFamilyTag, RoleFillersTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptRoleFillers");
    bind_view<Concept<runir::kr::BaseFamilyTag, OneOfTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptOneOf");
    bind_view<Concept<runir::kr::BaseFamilyTag, NominalTag>, runir::kr::dl::BaseConstructorRepository>(m, "ConceptNominal");

    bind_view<Role<runir::kr::BaseFamilyTag, UniversalTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleUniversal");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::BaseConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::BaseConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::BaseConstructorRepository>(m,
                                                                                                                                    "RoleAtomicStateDerived");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::BaseConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::BaseConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::BaseConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<Role<runir::kr::BaseFamilyTag, IntersectionTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleIntersection");
    bind_view<Role<runir::kr::BaseFamilyTag, UnionTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleUnion");
    bind_view<Role<runir::kr::BaseFamilyTag, ComplementTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleComplement");
    bind_view<Role<runir::kr::BaseFamilyTag, InverseTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleInverse");
    bind_view<Role<runir::kr::BaseFamilyTag, CompositionTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleComposition");
    bind_view<Role<runir::kr::BaseFamilyTag, TransitiveClosureTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<Role<runir::kr::BaseFamilyTag, ReflexiveTransitiveClosureTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<Role<runir::kr::BaseFamilyTag, RestrictionTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleRestriction");
    bind_view<Role<runir::kr::BaseFamilyTag, IdentityTag>, runir::kr::dl::BaseConstructorRepository>(m, "RoleIdentity");

    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "BooleanAtomicStateStatic");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "BooleanAtomicStateFluent");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "BooleanAtomicStateDerived");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::BaseConstructorRepository>(m,
                                                                                                                                     "BooleanAtomicGoalStatic");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::BaseConstructorRepository>(m,
                                                                                                                                     "BooleanAtomicGoalFluent");
    bind_view<Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::BaseConstructorRepository>(
        m,
        "BooleanAtomicGoalDerived");
    bind_view<Boolean<runir::kr::BaseFamilyTag, NonemptyTag>, runir::kr::dl::BaseConstructorRepository>(m, "BooleanNonempty");

    bind_view<Numerical<runir::kr::BaseFamilyTag, CountTag>, runir::kr::dl::BaseConstructorRepository>(m, "NumericalCount");
    bind_view<Numerical<runir::kr::BaseFamilyTag, DistanceTag>, runir::kr::dl::BaseConstructorRepository>(m, "NumericalDistance");

    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::BaseFamilyTag, ConceptTag, runir::kr::dl::BaseConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::BaseFamilyTag, RoleTag, runir::kr::dl::BaseConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::BaseFamilyTag, BooleanTag, runir::kr::dl::BaseConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::BaseFamilyTag, NumericalTag, runir::kr::dl::BaseConstructorRepository>,
          "constructor"_a);
}

}  // namespace runir::kr::dl::base
