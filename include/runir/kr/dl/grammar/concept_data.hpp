#ifndef RUNIR_GRAMMAR_CONCEPT_DATA_HPP_
#define RUNIR_GRAMMAR_CONCEPT_DATA_HPP_

#include "runir/kr/dl/data_helpers.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <concepts>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace runir::kr::dl::grammar
{

using GrammarConceptChoice = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>;

using GrammarRoleChoice = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>;

/// The arguments of count and nonempty in a grammar: a concept or a role choice.
using GrammarArgumentTypes = boost::mp11::mp_transform<runir::kr::dl::grammar::ConstructorOrNonTerminal, runir::kr::dl::ConceptOrRoleTags>;

}  // namespace runir::kr::dl::grammar

namespace urd
{

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::BotTag> || std::same_as<Tag, runir::kr::dl::TopTag>)
struct data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::NullaryData<runir::kr::dl::grammar::Concept<Tag>>
{
    using Base = runir::kr::dl::NullaryData<runir::kr::dl::grammar::Concept<Tag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct data<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>)
struct data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarConceptChoice, runir::kr::dl::grammar::GrammarConceptChoice>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarConceptChoice, runir::kr::dl::grammar::GrammarConceptChoice>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>, runir::kr::dl::grammar::GrammarConceptChoice>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>, runir::kr::dl::grammar::GrammarConceptChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
struct data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarConceptChoice>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarConceptChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
struct data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::NumberRestrictionData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarRoleChoice>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarRoleChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
struct data<runir::kr::dl::grammar::Concept<Tag>> :
    runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarConceptChoice>
{
    using Base = runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarConceptChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
struct data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarRoleChoice>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, runir::kr::dl::grammar::GrammarRoleChoice, runir::kr::dl::grammar::GrammarRoleChoice>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::RoleFillersData<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>, runir::kr::dl::grammar::GrammarRoleChoice>
{
    using Base = runir::kr::dl::RoleFillersData<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>, runir::kr::dl::grammar::GrammarRoleChoice>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>> : runir::kr::dl::ObjectListData<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::ObjectListData<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>> : runir::kr::dl::ObjectData<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::ObjectData<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Concept<runir::kr::dl::RegisterTag>> :
    runir::kr::dl::RegisterData<runir::kr::dl::grammar::Concept<runir::kr::dl::RegisterTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base =
        runir::kr::dl::RegisterData<runir::kr::dl::grammar::Concept<runir::kr::dl::RegisterTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct data<runir::kr::dl::grammar::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> :
    runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                             runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace urd

#endif
