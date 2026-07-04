#include "module.hpp"

#include <concepts>
#include <nanobind/stl/string.h>
#include <runir/kr/dl/declarations.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <runir/kr/dl/semantics/syntactic_complexity.hpp>
#include <runir/kr/dl/semantics/uns/evaluation.hpp>
#include <runir/kr/dl/semantics/uns/evaluation_context.hpp>
#include <runir/kr/dl/semantics/views.hpp>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

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
    if constexpr (requires(const View& view) { view.get_arg(); })
        cls.def("get_arg", &View::get_arg, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_lhs(); })
        cls.def("get_lhs", &View::get_lhs, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_mid(); })
        cls.def("get_mid", &View::get_mid, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_rhs(); })
        cls.def("get_rhs", &View::get_rhs, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_value(); })
        cls.def("get_value", &View::get_value);
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
        cls.def("evaluate", nb::overload_cast<View, GroundContext&>(&runir::kr::dl::semantics::evaluate), "context"_a);
    if constexpr (requires(const View& view, LiftedContext& context) { runir::kr::dl::semantics::evaluate(view, context); })
        cls.def("evaluate", nb::overload_cast<View, LiftedContext&>(&runir::kr::dl::semantics::evaluate), "context"_a);
    if constexpr (requires(const View& view) { runir::kr::dl::semantics::syntactic_complexity(view); })
        cls.def("syntactic_complexity", nb::overload_cast<View>(&runir::kr::dl::semantics::syntactic_complexity));
}

}  // namespace

void bind_semantics_views(nb::module_& m)
{
    bind_view<Constructor<runir::kr::UnsFamilyTag, ConceptTag>, runir::kr::dl::UnsConstructorRepository>(m, "Concept");
    bind_view<Constructor<runir::kr::UnsFamilyTag, RoleTag>, runir::kr::dl::UnsConstructorRepository>(m, "Role");
    bind_view<Constructor<runir::kr::UnsFamilyTag, BooleanTag>, runir::kr::dl::UnsConstructorRepository>(m, "Boolean");
    bind_view<Constructor<runir::kr::UnsFamilyTag, NumericalTag>, runir::kr::dl::UnsConstructorRepository>(m, "Numerical");

    bind_view<Concept<runir::kr::UnsFamilyTag, BotTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptBot");
    bind_view<Concept<runir::kr::UnsFamilyTag, TopTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptTop");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                    "ConceptAtomicStateStatic");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                    "ConceptAtomicStateFluent");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::UnsConstructorRepository>(
        m,
        "ConceptAtomicStateDerived");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                   "ConceptAtomicGoalStatic");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                   "ConceptAtomicGoalFluent");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                    "ConceptAtomicGoalDerived");
    bind_view<Concept<runir::kr::UnsFamilyTag, IntersectionTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptIntersection");
    bind_view<Concept<runir::kr::UnsFamilyTag, UnionTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptUnion");
    bind_view<Concept<runir::kr::UnsFamilyTag, NegationTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptNegation");
    bind_view<Concept<runir::kr::UnsFamilyTag, ValueRestrictionTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptValueRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, ExistentialQuantificationTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtLeastNumberRestrictionTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, AtMostNumberRestrictionTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptAtMostNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, ExactNumberRestrictionTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptExactNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, QualifiedAtLeastNumberRestrictionTag>, runir::kr::dl::UnsConstructorRepository>(
        m,
        "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, QualifiedAtMostNumberRestrictionTag>, runir::kr::dl::UnsConstructorRepository>(
        m,
        "ConceptQualifiedAtMostNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, QualifiedExactNumberRestrictionTag>, runir::kr::dl::UnsConstructorRepository>(
        m,
        "ConceptQualifiedExactNumberRestriction");
    bind_view<Concept<runir::kr::UnsFamilyTag, RoleValueMapTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptRoleValueMap");
    bind_view<Concept<runir::kr::UnsFamilyTag, AgreementTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptAgreement");
    bind_view<Concept<runir::kr::UnsFamilyTag, RoleFillersTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptRoleFillers");
    bind_view<Concept<runir::kr::UnsFamilyTag, OneOfTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptOneOf");
    bind_view<Concept<runir::kr::UnsFamilyTag, NominalTag>, runir::kr::dl::UnsConstructorRepository>(m, "ConceptNominal");

    bind_view<Role<runir::kr::UnsFamilyTag, UniversalTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleUniversal");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::UnsConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::UnsConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::UnsConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::UnsConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::UnsConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<Role<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::UnsConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<Role<runir::kr::UnsFamilyTag, IntersectionTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleIntersection");
    bind_view<Role<runir::kr::UnsFamilyTag, UnionTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleUnion");
    bind_view<Role<runir::kr::UnsFamilyTag, ComplementTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleComplement");
    bind_view<Role<runir::kr::UnsFamilyTag, InverseTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleInverse");
    bind_view<Role<runir::kr::UnsFamilyTag, CompositionTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleComposition");
    bind_view<Role<runir::kr::UnsFamilyTag, TransitiveClosureTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleTransitiveClosure");
    bind_view<Role<runir::kr::UnsFamilyTag, ReflexiveTransitiveClosureTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<Role<runir::kr::UnsFamilyTag, RestrictionTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleRestriction");
    bind_view<Role<runir::kr::UnsFamilyTag, IdentityTag>, runir::kr::dl::UnsConstructorRepository>(m, "RoleIdentity");

    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                    "BooleanAtomicStateStatic");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                    "BooleanAtomicStateFluent");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>, runir::kr::dl::UnsConstructorRepository>(
        m,
        "BooleanAtomicStateDerived");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                   "BooleanAtomicGoalStatic");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                   "BooleanAtomicGoalFluent");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>, runir::kr::dl::UnsConstructorRepository>(m,
                                                                                                                                    "BooleanAtomicGoalDerived");
    bind_view<Boolean<runir::kr::UnsFamilyTag, NonemptyTag>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanNonempty");
    bind_view<Boolean<runir::kr::UnsFamilyTag, EqTag<BooleanTag>>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanEq");
    bind_view<Boolean<runir::kr::UnsFamilyTag, NeqTag<BooleanTag>>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanNeq");
    bind_view<Boolean<runir::kr::UnsFamilyTag, LtTag<BooleanTag>>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanLt");
    bind_view<Boolean<runir::kr::UnsFamilyTag, LeTag<BooleanTag>>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanLe");
    bind_view<Boolean<runir::kr::UnsFamilyTag, GtTag<BooleanTag>>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanGt");
    bind_view<Boolean<runir::kr::UnsFamilyTag, GeTag<BooleanTag>>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanGe");
    bind_view<Boolean<runir::kr::UnsFamilyTag, EqTag<NumericalTag>>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalEq");
    bind_view<Boolean<runir::kr::UnsFamilyTag, NeqTag<NumericalTag>>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalNeq");
    bind_view<Boolean<runir::kr::UnsFamilyTag, LtTag<NumericalTag>>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalLt");
    bind_view<Boolean<runir::kr::UnsFamilyTag, LeTag<NumericalTag>>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalLe");
    bind_view<Boolean<runir::kr::UnsFamilyTag, GtTag<NumericalTag>>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalGt");
    bind_view<Boolean<runir::kr::UnsFamilyTag, GeTag<NumericalTag>>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalGe");
    bind_view<Boolean<runir::kr::UnsFamilyTag, BooleanConstantTag>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanConstant");
    bind_view<Boolean<runir::kr::UnsFamilyTag, AndTag>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanAnd");
    bind_view<Boolean<runir::kr::UnsFamilyTag, OrTag>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanOr");
    bind_view<Boolean<runir::kr::UnsFamilyTag, NotTag>, runir::kr::dl::UnsConstructorRepository>(m, "BooleanNot");

    bind_view<Numerical<runir::kr::UnsFamilyTag, CountTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalCount");
    bind_view<Numerical<runir::kr::UnsFamilyTag, DistanceTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalDistance");
    bind_view<Numerical<runir::kr::UnsFamilyTag, NumericalConstantTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalConstant");
    bind_view<Numerical<runir::kr::UnsFamilyTag, AddTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalAdd");
    bind_view<Numerical<runir::kr::UnsFamilyTag, SubTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalSub");
    bind_view<Numerical<runir::kr::UnsFamilyTag, MulTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalMul");
    bind_view<Numerical<runir::kr::UnsFamilyTag, DivTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalDiv");
    bind_view<Numerical<runir::kr::UnsFamilyTag, MinTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalMin");
    bind_view<Numerical<runir::kr::UnsFamilyTag, MaxTag>, runir::kr::dl::UnsConstructorRepository>(m, "NumericalMax");

    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::UnsFamilyTag, ConceptTag, runir::kr::dl::UnsConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::UnsFamilyTag, RoleTag, runir::kr::dl::UnsConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::UnsFamilyTag, BooleanTag, runir::kr::dl::UnsConstructorRepository>,
          "constructor"_a);
    m.def("syntactic_complexity",
          &runir::kr::dl::semantics::syntactic_complexity<runir::kr::UnsFamilyTag, NumericalTag, runir::kr::dl::UnsConstructorRepository>,
          "constructor"_a);
}

}  // namespace runir::kr::dl::uns
