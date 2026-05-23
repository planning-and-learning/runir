#ifndef RUNIR_CNF_GRAMMAR_CONCEPT_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_CONCEPT_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"
#include "runir/kr/dl/ext/register_index.hpp"

namespace tyr
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::BotTag>> :
    runir::kr::dl::cnf_grammar::NullaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::BotTag>>
{
    using Base = runir::kr::dl::cnf_grammar::NullaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::BotTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::TopTag>> :
    runir::kr::dl::cnf_grammar::NullaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::TopTag>>
{
    using Base = runir::kr::dl::cnf_grammar::NullaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::TopTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::IntersectionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::IntersectionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::UnionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::UnionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::UnionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::NegationTag>> :
    runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::NegationTag>,
                                          runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::UnaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::NegationTag>,
                                                       runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ValueRestrictionTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ValueRestrictionTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ValueRestrictionTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>> :
    runir::kr::dl::cnf_grammar::NumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>>
{
    using Base =
        runir::kr::dl::cnf_grammar::NumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>> :
    runir::kr::dl::cnf_grammar::NumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>>
{
    using Base =
        runir::kr::dl::cnf_grammar::NumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>> :
    runir::kr::dl::cnf_grammar::NumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>>
{
    using Base =
        runir::kr::dl::cnf_grammar::NumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>> :
    runir::kr::dl::cnf_grammar::QualifiedNumberRestrictionData<Family,
                                                               runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>
{
    using Base = runir::kr::dl::cnf_grammar::
        QualifiedNumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>> :
    runir::kr::dl::cnf_grammar::QualifiedNumberRestrictionData<Family,
                                                               runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>
{
    using Base = runir::kr::dl::cnf_grammar::
        QualifiedNumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedExactNumberRestrictionTag>> :
    runir::kr::dl::cnf_grammar::QualifiedNumberRestrictionData<Family,
                                                               runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedExactNumberRestrictionTag>>
{
    using Base = runir::kr::dl::cnf_grammar::
        QualifiedNumberRestrictionData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::QualifiedExactNumberRestrictionTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RoleValueMapTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RoleValueMapTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RoleValueMapTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AgreementTag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AgreementTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::AgreementTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::cnf_grammar::RoleFillersData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RoleFillersTag>>
{
    using Base = runir::kr::dl::cnf_grammar::RoleFillersData<Family, runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RoleFillersTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::OneOfTag>> :
    runir::kr::dl::cnf_grammar::ObjectListData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::cnf_grammar::ObjectListData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::NominalTag>> :
    runir::kr::dl::cnf_grammar::ObjectData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::cnf_grammar::ObjectData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::NominalTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::semantics::RegisterData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RegisterTag>,
                                           runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::RegisterData<runir::kr::dl::cnf_grammar::Concept<Family, runir::kr::dl::RegisterTag>,
                                                        runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
