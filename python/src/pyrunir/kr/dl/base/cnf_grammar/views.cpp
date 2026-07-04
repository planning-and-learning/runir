#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/cnf_grammar/constructor_repository.hpp>
#include <runir/kr/dl/cnf_grammar/formatter.hpp>
#include <runir/kr/dl/cnf_grammar/views.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::dl::base
{

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
    if constexpr (requires(const View& view) { view.get_name(); })
        cls.def("get_name", &View::get_name);
    if constexpr (requires(const View& view) { view.get_variant(); })
        cls.def("get_variant", &View::get_variant, nb::rv_policy::reference_internal);
}

}  // namespace

void bind_cnf_grammar_views(nb::module_& m)
{
    bind_view<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, ConceptTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "Concept");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, RoleTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "Role");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, BooleanTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "Boolean");
    bind_view<runir::kr::dl::cnf_grammar::Constructor<runir::kr::BaseFamilyTag, NumericalTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "Numerical");
    bind_view<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, ConceptTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptNonTerminal");
    bind_view<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, RoleTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleNonTerminal");
    bind_view<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, BooleanTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "BooleanNonTerminal");
    bind_view<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::BaseFamilyTag, NumericalTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "NumericalNonTerminal");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, BotTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptBot");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, TopTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptTop");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAtomicGoalStatic");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAtomicGoalFluent");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAtomicGoalDerived");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, IntersectionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptIntersection");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, UnionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptUnion");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, NegationTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptNegation");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ValueRestrictionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptValueRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ExistentialQuantificationTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptExistentialQuantification");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtLeastNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAtLeastNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AtMostNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAtMostNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, ExactNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptExactNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtLeastNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptQualifiedAtLeastNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedAtMostNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptQualifiedAtMostNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, QualifiedExactNumberRestrictionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptQualifiedExactNumberRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, RoleValueMapTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptRoleValueMap");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, AgreementTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptAgreement");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, RoleFillersTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptRoleFillers");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, OneOfTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptOneOf");
    bind_view<runir::kr::dl::cnf_grammar::Concept<runir::kr::BaseFamilyTag, NominalTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "ConceptNominal");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, UniversalTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleUniversal");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleAtomicGoalStatic");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleAtomicGoalFluent");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleAtomicGoalDerived");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, IntersectionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleIntersection");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, UnionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleUnion");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, ComplementTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleComplement");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, InverseTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleInverse");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, CompositionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleComposition");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, TransitiveClosureTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleTransitiveClosure");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, ReflexiveTransitiveClosureTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleReflexiveTransitiveClosure");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, RestrictionTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleRestriction");
    bind_view<runir::kr::dl::cnf_grammar::Role<runir::kr::BaseFamilyTag, IdentityTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "RoleIdentity");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "BooleanAtomicStateStatic");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "BooleanAtomicStateFluent");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicStateTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "BooleanAtomicStateDerived");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::StaticTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "BooleanAtomicGoalStatic");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::FluentTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "BooleanAtomicGoalFluent");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, AtomicGoalTag<tyr::formalism::DerivedTag>>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "BooleanAtomicGoalDerived");
    bind_view<runir::kr::dl::cnf_grammar::Boolean<runir::kr::BaseFamilyTag, NonemptyTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "BooleanNonempty");
    bind_view<runir::kr::dl::cnf_grammar::Numerical<runir::kr::BaseFamilyTag, CountTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "NumericalCount");
    bind_view<runir::kr::dl::cnf_grammar::Numerical<runir::kr::BaseFamilyTag, DistanceTag>,
              runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>>(m, "NumericalDistance");
}

}  // namespace runir::kr::dl::base
