#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/base/repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/views.hpp>
#include <tyr/common/python/bindings.hpp>
#include <tyr/common/python/type_casters.hpp>

namespace runir::kr::dl::base
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
    if constexpr (requires(const View& view) { view.get_objects(); })
        cls.def("get_objects", &View::get_objects);
    if constexpr (requires(const View& view) { view.get_n(); })
        cls.def("get_n", &View::get_n);
    if constexpr (requires(const View& view) { view.get_role(); })
        cls.def("get_role", &View::get_role);
    if constexpr (requires(const View& view) { view.get_concept(); })
        cls.def("get_concept", &View::get_concept);
    if constexpr (requires(const View& view) { view.get_name(); })
        cls.def("get_name", &View::get_name);
    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant);
}

}  // namespace

void bind_cnf_grammar_views(nb::module_& m)
{
    bind_view<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, ConceptTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "Concept");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, RoleTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "Role");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, BooleanTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "Boolean");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, NumericalTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "Numerical");
    bind_view<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, ConceptTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "ConceptNonTerminal");
    bind_view<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, RoleTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "RoleNonTerminal");
    bind_view<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, BooleanTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "BooleanNonTerminal");
    bind_view<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, NumericalTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "NumericalNonTerminal");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, BotTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptBot");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, TopTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptTop");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptAtomicGoalStatic");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptAtomicGoalFluent");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptAtomicGoalDerived");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, IntersectionTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "ConceptIntersection");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, UnionTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m,
                                                                                                                                                "ConceptUnion");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, NegationTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "ConceptNegation");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ValueRestrictionTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "ConceptValueRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ExistentialQuantificationTag>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptExistentialQuantification");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtLeastNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptAtLeastNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtMostNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptAtMostNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ExactNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptExactNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptQualifiedAtMostNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedExactNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "ConceptQualifiedExactNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, RoleValueMapTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "ConceptRoleValueMap");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AgreementTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "ConceptAgreement");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, RoleFillersTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "ConceptRoleFillers");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, OneOfTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m,
                                                                                                                                                "ConceptOneOf");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, NominalTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "ConceptNominal");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, UniversalTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "RoleUniversal");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "RoleAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "RoleAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "RoleAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "RoleAtomicGoalStatic");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "RoleAtomicGoalFluent");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "RoleAtomicGoalDerived");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, IntersectionTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "RoleIntersection");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, UnionTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "RoleUnion");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, ComplementTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "RoleComplement");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, InverseTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m,
                                                                                                                                               "RoleInverse");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, CompositionTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "RoleComposition");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, TransitiveClosureTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "RoleTransitiveClosure");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, ReflexiveTransitiveClosureTag>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "RoleReflexiveTransitiveClosure");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, RestrictionTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "RoleRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, IdentityTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m,
                                                                                                                                                "RoleIdentity");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "BooleanAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "BooleanAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "BooleanAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "BooleanAtomicGoalStatic");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "BooleanAtomicGoalFluent");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::base::ConstructorRepository>(m, "BooleanAtomicGoalDerived");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, NonemptyTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "BooleanNonempty");
    bind_view<runir::kr::dl::cnf_grammar::Numerical<runir::kr::BaseFamilyTag, CountTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "NumericalCount");
    bind_view<runir::kr::dl::cnf_grammar::Numerical<runir::kr::BaseFamilyTag, DistanceTag>, runir::kr::dl::cnf_grammar::base::ConstructorRepository>(
        m,
        "NumericalDistance");
}

}  // namespace runir::kr::dl::base
