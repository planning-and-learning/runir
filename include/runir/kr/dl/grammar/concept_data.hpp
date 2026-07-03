#ifndef RUNIR_GRAMMAR_CONCEPT_DATA_HPP_
#define RUNIR_GRAMMAR_CONCEPT_DATA_HPP_

#include "runir/kr/dl/grammar/indices.hpp"
#include "runir/kr/dl/semantics/data_helpers.hpp"

#include <concepts>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
using GrammarConceptChoice = runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using GrammarRoleChoice = runir::kr::dl::grammar::ConstructorOrNonTerminal<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family>
using GrammarConceptConstructor = runir::kr::dl::grammar::Constructor<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using GrammarRoleConstructor = runir::kr::dl::grammar::Constructor<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::BotTag> || std::same_as<Tag, runir::kr::dl::TopTag>)
struct Data<runir::kr::dl::grammar::Concept<Family, Tag>> : runir::kr::dl::semantics::NullaryData<runir::kr::dl::grammar::Concept<Family, Tag>>
{
    using Base = runir::kr::dl::semantics::NullaryData<runir::kr::dl::grammar::Concept<Family, Tag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>)
struct Data<runir::kr::dl::grammar::Concept<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarConceptChoice<Family>, GrammarConceptChoice<Family>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarConceptChoice<Family>, GrammarConceptChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::NegationTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::NegationTag>, GrammarConceptChoice<Family>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::NegationTag>, GrammarConceptChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::ValueRestrictionTag> || std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
struct Data<runir::kr::dl::grammar::Concept<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarRoleChoice<Family>, GrammarConceptChoice<Family>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarRoleChoice<Family>, GrammarConceptChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::AtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::AtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::ExactNumberRestrictionTag>)
struct Data<runir::kr::dl::grammar::Concept<Family, Tag>> :
    runir::kr::dl::semantics::NumberRestrictionData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarRoleChoice<Family>>
{
    using Base = runir::kr::dl::semantics::NumberRestrictionData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarRoleChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
struct Data<runir::kr::dl::grammar::Concept<Family, Tag>> :
    runir::kr::dl::semantics::
        QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarRoleChoice<Family>, GrammarConceptChoice<Family>>
{
    using Base = runir::kr::dl::semantics::
        QualifiedNumberRestrictionData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarRoleChoice<Family>, GrammarConceptChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
struct Data<runir::kr::dl::grammar::Concept<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarRoleChoice<Family>, GrammarRoleChoice<Family>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::grammar::Concept<Family, Tag>, GrammarRoleChoice<Family>, GrammarRoleChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::semantics::RoleFillersData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::RoleFillersTag>, GrammarRoleChoice<Family>>
{
    using Base = runir::kr::dl::semantics::RoleFillersData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::RoleFillersTag>, GrammarRoleChoice<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::OneOfTag>> :
    runir::kr::dl::semantics::ObjectListData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::semantics::ObjectListData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::NominalTag>> :
    runir::kr::dl::semantics::ObjectData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::semantics::ObjectData<runir::kr::dl::grammar::Concept<Family, runir::kr::dl::NominalTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::semantics::RegisterData<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                           runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::RegisterData<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                                        runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> :
    runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                           runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>
{
    using Base =
        runir::kr::dl::semantics::ArgumentData<runir::kr::dl::grammar::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                               runir::kr::dl::ArgumentIdentifier<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
