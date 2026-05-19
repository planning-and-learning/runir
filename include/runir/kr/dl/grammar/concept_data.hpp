#ifndef RUNIR_GRAMMAR_CONCEPT_DATA_HPP_
#define RUNIR_GRAMMAR_CONCEPT_DATA_HPP_

#include "runir/kr/dl/grammar/data_helpers.hpp"

namespace tyr
{

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::BotTag>> :
    runir::kr::dl::grammar::NullaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::BotTag>>
{
    using Base = runir::kr::dl::grammar::NullaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::BotTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::TopTag>> :
    runir::kr::dl::grammar::NullaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::TopTag>>
{
    using Base = runir::kr::dl::grammar::NullaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::TopTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::grammar::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::IntersectionTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::IntersectionTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::UnionTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::UnionTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::UnionTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>> :
    runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>,
                                      runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::grammar::UnaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>,
                                                   runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::ValueRestrictionTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::ValueRestrictionTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::ValueRestrictionTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::ExistentialQuantificationTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::ExistentialQuantificationTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::ExistentialQuantificationTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::AtLeastNumberRestrictionTag>> :
    runir::kr::dl::grammar::NumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtLeastNumberRestrictionTag>>
{
    using Base = runir::kr::dl::grammar::NumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtLeastNumberRestrictionTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::AtMostNumberRestrictionTag>> :
    runir::kr::dl::grammar::NumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtMostNumberRestrictionTag>>
{
    using Base = runir::kr::dl::grammar::NumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtMostNumberRestrictionTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::ExactNumberRestrictionTag>> :
    runir::kr::dl::grammar::NumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::ExactNumberRestrictionTag>>
{
    using Base = runir::kr::dl::grammar::NumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::ExactNumberRestrictionTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>> :
    runir::kr::dl::grammar::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>
{
    using Base = runir::kr::dl::grammar::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>> :
    runir::kr::dl::grammar::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>
{
    using Base = runir::kr::dl::grammar::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>> :
    runir::kr::dl::grammar::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>>
{
    using Base = runir::kr::dl::grammar::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<runir::kr::dl::QualifiedExactNumberRestrictionTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleValueMapTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleValueMapTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleValueMapTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::AgreementTag>> :
    runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::AgreementTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                       runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::grammar::BinaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::AgreementTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>,
                                                    runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::grammar::RoleFillersData<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>>
{
    using Base = runir::kr::dl::grammar::RoleFillersData<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>> :
    runir::kr::dl::grammar::ObjectListData<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::grammar::ObjectListData<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>> :
    runir::kr::dl::grammar::ObjectData<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::grammar::ObjectData<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
