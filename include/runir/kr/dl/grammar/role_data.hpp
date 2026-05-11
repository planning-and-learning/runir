#ifndef RUNIR_GRAMMAR_ROLE_DATA_HPP_
#define RUNIR_GRAMMAR_ROLE_DATA_HPP_

#include "runir/kr/dl/grammar/data_helpers.hpp"

namespace tyr
{

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::UniversalTag>> :
    runir::kr::dl::grammar::NullaryData<runir::kr::dl::grammar::Role<runir::kr::dl::UniversalTag>>
{
    using Base = runir::kr::dl::grammar::NullaryData<runir::kr::dl::grammar::Role<runir::kr::dl::UniversalTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Role<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::IntersectionTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::IntersectionTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::UnionTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::UnionTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::UnionTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::ComplementTag>> :
    runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::ComplementTag>,
                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::ComplementTag>,
                                                   runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::InverseTag>> :
    runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::InverseTag>,
                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::InverseTag>,
                                                   runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::CompositionTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::CompositionTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::CompositionTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::TransitiveClosureTag>> :
    runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::TransitiveClosureTag>,
                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::TransitiveClosureTag>,
                                                   runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>> :
    runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>,
                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>,
                                                   runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::RestrictionTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::RestrictionTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Role<runir::kr::dl::RestrictionTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Role<runir::kr::dl::IdentityTag>> :
    runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::IdentityTag>,
                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Role<runir::kr::dl::IdentityTag>,
                                                   runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
