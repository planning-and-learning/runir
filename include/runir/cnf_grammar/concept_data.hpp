#ifndef RUNIR_CNF_GRAMMAR_CONCEPT_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_CONCEPT_DATA_HPP_

#include "runir/cnf_grammar/data_helpers.hpp"

namespace tyr
{

template<>
struct Data<runir::cnf_grammar::Concept<runir::BotTag>> : runir::cnf_grammar::NullaryData<runir::cnf_grammar::Concept<runir::BotTag>>
{
    using Base = runir::cnf_grammar::NullaryData<runir::cnf_grammar::Concept<runir::BotTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::TopTag>> : runir::cnf_grammar::NullaryData<runir::cnf_grammar::Concept<runir::TopTag>>
{
    using Base = runir::cnf_grammar::NullaryData<runir::cnf_grammar::Concept<runir::TopTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::cnf_grammar::Concept<runir::AtomicStateTag<T>>> : runir::cnf_grammar::PredicateData<runir::cnf_grammar::Concept<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::cnf_grammar::PredicateData<runir::cnf_grammar::Concept<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::cnf_grammar::Concept<runir::AtomicGoalTag<T>>> : runir::cnf_grammar::PredicateData<runir::cnf_grammar::Concept<runir::AtomicGoalTag<T>>, T>
{
    using Base = runir::cnf_grammar::PredicateData<runir::cnf_grammar::Concept<runir::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::IntersectionTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::IntersectionTag>,
                                   runir::cnf_grammar::NonTerminal<runir::ConceptTag>,
                                   runir::cnf_grammar::NonTerminal<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::IntersectionTag>,
                                                runir::cnf_grammar::NonTerminal<runir::ConceptTag>,
                                                runir::cnf_grammar::NonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::UnionTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::UnionTag>,
                                   runir::cnf_grammar::NonTerminal<runir::ConceptTag>,
                                   runir::cnf_grammar::NonTerminal<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::UnionTag>,
                                                runir::cnf_grammar::NonTerminal<runir::ConceptTag>,
                                                runir::cnf_grammar::NonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::NegationTag>> :
    runir::cnf_grammar::UnaryData<runir::cnf_grammar::Concept<runir::NegationTag>, runir::cnf_grammar::NonTerminal<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::UnaryData<runir::cnf_grammar::Concept<runir::NegationTag>, runir::cnf_grammar::NonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::ValueRestrictionTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::ValueRestrictionTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                   runir::cnf_grammar::NonTerminal<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::ValueRestrictionTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                                runir::cnf_grammar::NonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::ExistentialQuantificationTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::ExistentialQuantificationTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                   runir::cnf_grammar::NonTerminal<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::ExistentialQuantificationTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                                runir::cnf_grammar::NonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::RoleValueMapContainmentTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::RoleValueMapContainmentTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::RoleValueMapContainmentTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::RoleValueMapEqualityTag>> :
    runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::RoleValueMapEqualityTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                   runir::cnf_grammar::NonTerminal<runir::RoleTag>>
{
    using Base = runir::cnf_grammar::BinaryData<runir::cnf_grammar::Concept<runir::RoleValueMapEqualityTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                                runir::cnf_grammar::NonTerminal<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Concept<runir::NominalTag>> : runir::cnf_grammar::ObjectData<runir::cnf_grammar::Concept<runir::NominalTag>>
{
    using Base = runir::cnf_grammar::ObjectData<runir::cnf_grammar::Concept<runir::NominalTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
