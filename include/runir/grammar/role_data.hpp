#ifndef RUNIR_GRAMMAR_ROLE_DATA_HPP_
#define RUNIR_GRAMMAR_ROLE_DATA_HPP_

#include "runir/grammar/data_helpers.hpp"

namespace tyr
{

template<>
struct Data<runir::grammar::Role<runir::UniversalTag>> : runir::grammar::NullaryData<runir::grammar::Role<runir::UniversalTag>>
{
    using Base = runir::grammar::NullaryData<runir::grammar::Role<runir::UniversalTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::grammar::Role<runir::AtomicStateTag<T>>> : runir::grammar::PredicateData<runir::grammar::Role<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::grammar::PredicateData<runir::grammar::Role<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::grammar::Role<runir::AtomicGoalTag<T>>> : runir::grammar::PredicateData<runir::grammar::Role<runir::AtomicGoalTag<T>>, T>
{
    using Base = runir::grammar::PredicateData<runir::grammar::Role<runir::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::IntersectionTag>> :
    runir::grammar::BinaryData<runir::grammar::Role<runir::IntersectionTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Role<runir::IntersectionTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::UnionTag>> :
    runir::grammar::BinaryData<runir::grammar::Role<runir::UnionTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Role<runir::UnionTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::ComplementTag>> :
    runir::grammar::UnaryData<runir::grammar::Role<runir::ComplementTag>, runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base = runir::grammar::UnaryData<runir::grammar::Role<runir::ComplementTag>, runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::InverseTag>> :
    runir::grammar::UnaryData<runir::grammar::Role<runir::InverseTag>, runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base = runir::grammar::UnaryData<runir::grammar::Role<runir::InverseTag>, runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::CompositionTag>> :
    runir::grammar::BinaryData<runir::grammar::Role<runir::CompositionTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Role<runir::CompositionTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::TransitiveClosureTag>> :
    runir::grammar::UnaryData<runir::grammar::Role<runir::TransitiveClosureTag>, runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base = runir::grammar::UnaryData<runir::grammar::Role<runir::TransitiveClosureTag>, runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::ReflexiveTransitiveClosureTag>> :
    runir::grammar::UnaryData<runir::grammar::Role<runir::ReflexiveTransitiveClosureTag>, runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base =
        runir::grammar::UnaryData<runir::grammar::Role<runir::ReflexiveTransitiveClosureTag>, runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::RestrictionTag>> :
    runir::grammar::BinaryData<runir::grammar::Role<runir::RestrictionTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Role<runir::RestrictionTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Role<runir::IdentityTag>> :
    runir::grammar::UnaryData<runir::grammar::Role<runir::IdentityTag>, runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>
{
    using Base = runir::grammar::UnaryData<runir::grammar::Role<runir::IdentityTag>, runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
