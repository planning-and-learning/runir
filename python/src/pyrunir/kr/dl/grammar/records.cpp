#include "../record_bindings.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/grammar/constructor_repository.hpp>
#include <runir/kr/dl/grammar/formatter.hpp>
#include <urd/python/bindings.hpp>

namespace runir::kr::dl
{

void bind_grammar_records(nb::module_& m)
{
    using tyr::formalism::DerivedTag;
    using tyr::formalism::FluentTag;
    using tyr::formalism::StaticTag;

    urd::python::bind_data<Concept<grammar::GrammarFamily, BotTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, TopTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, IntersectionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, UnionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, NegationTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, ValueRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, ExistentialQuantificationTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtLeastNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AtMostNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, ExactNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, QualifiedAtLeastNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, QualifiedAtMostNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, QualifiedExactNumberRestrictionTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, RoleValueMapTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, AgreementTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, RoleFillersTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, OneOfTag>>(m);
    urd::python::bind_data<Concept<grammar::GrammarFamily, NominalTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, UniversalTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, IntersectionTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, UnionTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, ComplementTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, InverseTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, CompositionTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, TransitiveClosureTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, ReflexiveTransitiveClosureTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, RestrictionTag>>(m);
    urd::python::bind_data<Role<grammar::GrammarFamily, IdentityTag>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicStateTag<StaticTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicStateTag<FluentTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicStateTag<DerivedTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicGoalTag<StaticTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicGoalTag<FluentTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, AtomicGoalTag<DerivedTag>>>(m);
    urd::python::bind_data<Boolean<grammar::GrammarFamily, NonemptyTag>>(m);
    urd::python::bind_data<Numerical<grammar::GrammarFamily, CountTag>>(m);
    urd::python::bind_data<Numerical<grammar::GrammarFamily, DistanceTag>>(m);

    python::bind_record_views<grammar::GrammarFamily, grammar::ConstructorRepository>(m, ConceptTag {});
    python::bind_record_views<grammar::GrammarFamily, grammar::ConstructorRepository>(m, RoleTag {});
    python::bind_record_views<grammar::GrammarFamily, grammar::ConstructorRepository>(m, BooleanTag {});
    python::bind_record_views<grammar::GrammarFamily, grammar::ConstructorRepository>(m, NumericalTag {});
}

}  // namespace runir::kr::dl
