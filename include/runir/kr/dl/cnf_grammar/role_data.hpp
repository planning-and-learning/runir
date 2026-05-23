#ifndef RUNIR_CNF_GRAMMAR_ROLE_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_ROLE_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"

namespace tyr
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::UniversalTag>> :
    runir::kr::dl::cnf_grammar::NullaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::UniversalTag>>
{
    using Base = runir::kr::dl::cnf_grammar::NullaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::UniversalTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::IntersectionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::IntersectionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::UnionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::UnionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::UnionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::ComplementTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::ComplementTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::ComplementTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::InverseTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::InverseTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::InverseTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::CompositionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::CompositionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::CompositionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::TransitiveClosureTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::TransitiveClosureTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::TransitiveClosureTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::ReflexiveTransitiveClosureTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::ReflexiveTransitiveClosureTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::ReflexiveTransitiveClosureTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::RestrictionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::RestrictionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::RestrictionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::IdentityTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::IdentityTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::IdentityTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, runir::kr::dl::RegisterTag>
struct Data<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::cnf_grammar::RegisterData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::RegisterTag>,
                                             runir::kr::dl::Role<Family, runir::kr::dl::RegisterIdentifierTag>>
{
    using Base = runir::kr::dl::cnf_grammar::RegisterData<runir::kr::dl::cnf_grammar::Role<Family, runir::kr::dl::RegisterTag>,
                                                          runir::kr::dl::Role<Family, runir::kr::dl::RegisterIdentifierTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
