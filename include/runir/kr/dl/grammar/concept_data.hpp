#ifndef RUNIR_GRAMMAR_CONCEPT_DATA_HPP_
#define RUNIR_GRAMMAR_CONCEPT_DATA_HPP_

#include "runir/kr/dl/data_helpers.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <concepts>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

using GrammarConceptChoice = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::ConceptTag>;

using GrammarRoleChoice = runir::kr::dl::grammar::ConstructorOrNonTerminal<runir::kr::dl::RoleTag>;

/// The arguments of count and nonempty in a grammar: a concept or a role choice.
using GrammarArgumentTypes = ygg::MapTypeListT<runir::kr::dl::grammar::ConstructorOrNonTerminal, runir::kr::dl::ConceptOrRoleTags>;

using GrammarConceptConstructor = runir::kr::dl::grammar::Constructor<runir::kr::dl::ConceptTag>;

using GrammarRoleConstructor = runir::kr::dl::grammar::Constructor<runir::kr::dl::RoleTag>;

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::BotTag> || std::same_as<Tag, runir::kr::dl::TopTag>)
struct Data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::NullaryData<runir::kr::dl::grammar::Concept<Tag>>
{
    using Base = runir::kr::dl::NullaryData<runir::kr::dl::grammar::Concept<Tag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::grammar::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>)
struct Data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, GrammarConceptChoice, GrammarConceptChoice>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, GrammarConceptChoice, GrammarConceptChoice>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>, GrammarConceptChoice>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::grammar::Concept<runir::kr::dl::NegationTag>, GrammarConceptChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
struct Data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, GrammarRoleChoice, GrammarConceptChoice>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, GrammarRoleChoice, GrammarConceptChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
struct Data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::NumberRestrictionData<runir::kr::dl::grammar::Concept<Tag>, GrammarRoleChoice>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::grammar::Concept<Tag>, GrammarRoleChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
struct Data<runir::kr::dl::grammar::Concept<Tag>> :
    runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<Tag>, GrammarRoleChoice, GrammarConceptChoice>
{
    using Base = runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<Tag>, GrammarRoleChoice, GrammarConceptChoice>;
    using Base::Base;
};

template<typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
struct Data<runir::kr::dl::grammar::Concept<Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, GrammarRoleChoice, GrammarRoleChoice>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::grammar::Concept<Tag>, GrammarRoleChoice, GrammarRoleChoice>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::RoleFillersData<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>, GrammarRoleChoice>
{
    using Base = runir::kr::dl::RoleFillersData<runir::kr::dl::grammar::Concept<runir::kr::dl::RoleFillersTag>, GrammarRoleChoice>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>> : runir::kr::dl::ObjectListData<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::ObjectListData<runir::kr::dl::grammar::Concept<runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>> : runir::kr::dl::ObjectData<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::ObjectData<runir::kr::dl::grammar::Concept<runir::kr::dl::NominalTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::RegisterTag>> :
    runir::kr::dl::RegisterData<runir::kr::dl::grammar::Concept<runir::kr::dl::RegisterTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base =
        runir::kr::dl::RegisterData<runir::kr::dl::grammar::Concept<runir::kr::dl::RegisterTag>, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> :
    runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::ArgumentData<runir::kr::dl::grammar::Concept<runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                             runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
