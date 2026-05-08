#ifndef RUNIR_CNF_GRAMMAR_ROLE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_ROLE_DATA_HPP_

#include "runir/cnf_grammar/data_helpers.hpp"

namespace tyr
{

template<>
struct Data<runir::cnf_grammar::Role<runir::UniversalTag>> : runir::cnf_grammar::NullaryData<runir::cnf_grammar::Role<runir::UniversalTag>>
{
    using Base = runir::cnf_grammar::NullaryData<runir::cnf_grammar::Role<runir::UniversalTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::cnf_grammar::Role<runir::AtomicStateTag<T>>> : runir::cnf_grammar::PredicateData<runir::cnf_grammar::Role<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::cnf_grammar::PredicateData<runir::cnf_grammar::Role<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::cnf_grammar::Role<runir::AtomicGoalTag<T>>> : runir::cnf_grammar::PredicateData<runir::cnf_grammar::Role<runir::AtomicGoalTag<T>>, T>
{
    using Base = runir::cnf_grammar::PredicateData<runir::cnf_grammar::Role<runir::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::IntersectionTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Role<runir::IntersectionTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Role<runir::IntersectionTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::UnionTag>> :
    runir::cnf_grammar::
        BinaryData<runir::cnf_grammar::Role<runir::UnionTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::
        BinaryData<runir::cnf_grammar::Role<runir::UnionTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::ComplementTag>> :
    runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::ComplementTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::ComplementTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::InverseTag>> :
    runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::InverseTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::InverseTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::CompositionTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Role<runir::CompositionTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Role<runir::CompositionTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::TransitiveClosureTag>> :
    runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::TransitiveClosureTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::TransitiveClosureTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::ReflexiveTransitiveClosureTag>> :
    runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::ReflexiveTransitiveClosureTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::ReflexiveTransitiveClosureTag>, runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::RestrictionTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Role<runir::RestrictionTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                   runir::cnf_grammar::NonTerminal<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Role<runir::RestrictionTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                                runir::cnf_grammar::NonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Role<runir::IdentityTag>> :
    runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::IdentityTag>, runir::cnf_grammar::NonTerminal<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::UnaryData<runir::cnf_grammar::Role<runir::IdentityTag>, runir::cnf_grammar::NonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
