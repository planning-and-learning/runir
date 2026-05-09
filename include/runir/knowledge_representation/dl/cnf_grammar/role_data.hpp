#ifndef RUNIR_CNF_GRAMMAR_ROLE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_ROLE_DATA_HPP_

#include "runir/knowledge_representation/dl/cnf_grammar/data_helpers.hpp"

namespace tyr
{

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::UniversalTag>> :
    runir::kr::dl::cnf_grammar::NullaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::UniversalTag>>
{
    using Base = runir::kr::dl::cnf_grammar::NullaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::UniversalTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::IntersectionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::IntersectionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::UnionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::UnionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::UnionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::ComplementTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::ComplementTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::ComplementTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::InverseTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::InverseTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::InverseTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::CompositionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::CompositionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::CompositionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::TransitiveClosureTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::TransitiveClosureTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::TransitiveClosureTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::RestrictionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::RestrictionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::RestrictionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::IdentityTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::IdentityTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<runir::kr::dl::IdentityTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
