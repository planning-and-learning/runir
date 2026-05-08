#ifndef RUNIR_GRAMMAR_CONCEPT_DATA_HPP_
#define RUNIR_GRAMMAR_CONCEPT_DATA_HPP_

#include "runir/grammar/data_helpers.hpp"

namespace tyr
{

template<>
struct Data<runir::grammar::Concept<runir::BotTag>> : runir::grammar::NullaryData<runir::grammar::Concept<runir::BotTag>>
{
    using Base = runir::grammar::NullaryData<runir::grammar::Concept<runir::BotTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::TopTag>> : runir::grammar::NullaryData<runir::grammar::Concept<runir::TopTag>>
{
    using Base = runir::grammar::NullaryData<runir::grammar::Concept<runir::TopTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::grammar::Concept<runir::AtomicStateTag<T>>> : runir::grammar::PredicateData<runir::grammar::Concept<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::grammar::PredicateData<runir::grammar::Concept<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::grammar::Concept<runir::AtomicGoalTag<T>>> : runir::grammar::PredicateData<runir::grammar::Concept<runir::AtomicGoalTag<T>>, T>
{
    using Base = runir::grammar::PredicateData<runir::grammar::Concept<runir::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::IntersectionTag>> :
    runir::grammar::BinaryData<runir::grammar::Concept<runir::IntersectionTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Concept<runir::IntersectionTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::UnionTag>> :
    runir::grammar::BinaryData<runir::grammar::Concept<runir::UnionTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Concept<runir::UnionTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::NegationTag>> :
    runir::grammar::UnaryData<runir::grammar::Concept<runir::NegationTag>, runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>
{
    using Base = runir::grammar::UnaryData<runir::grammar::Concept<runir::NegationTag>, runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::ValueRestrictionTag>> :
    runir::grammar::BinaryData<runir::grammar::Concept<runir::ValueRestrictionTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Concept<runir::ValueRestrictionTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::ExistentialQuantificationTag>> :
    runir::grammar::BinaryData<runir::grammar::Concept<runir::ExistentialQuantificationTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Concept<runir::ExistentialQuantificationTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::RoleValueMapContainmentTag>> :
    runir::grammar::BinaryData<runir::grammar::Concept<runir::RoleValueMapContainmentTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Concept<runir::RoleValueMapContainmentTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::RoleValueMapEqualityTag>> :
    runir::grammar::BinaryData<runir::grammar::Concept<runir::RoleValueMapEqualityTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                               runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>
{
    using Base = runir::grammar::BinaryData<runir::grammar::Concept<runir::RoleValueMapEqualityTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>,
                                            runir::grammar::ConstructorOrNonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Concept<runir::NominalTag>> : runir::grammar::ObjectData<runir::grammar::Concept<runir::NominalTag>>
{
    using Base = runir::grammar::ObjectData<runir::grammar::Concept<runir::NominalTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
