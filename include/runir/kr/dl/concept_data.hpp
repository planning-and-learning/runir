#ifndef RUNIR_KR_DL_CONCEPT_DATA_HPP_
#define RUNIR_KR_DL_CONCEPT_DATA_HPP_

#include "runir/kr/dl/data_helpers.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
using DlConcept = runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using DlRole = runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>;

/// The arguments of count and nonempty: a concept, a role, or a query.
template<runir::kr::dl::FamilyTag Family>
using DlArgumentTypes = ygg::TypeList<DlConcept<Family>, DlRole<Family>, runir::kr::dl::Query<Family>>;

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::BotTag>> : runir::kr::dl::NullaryData<runir::kr::dl::Concept<Family, runir::kr::dl::BotTag>>
{
    using Base = runir::kr::dl::NullaryData<runir::kr::dl::Concept<Family, runir::kr::dl::BotTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::TopTag>> : runir::kr::dl::NullaryData<runir::kr::dl::Concept<Family, runir::kr::dl::TopTag>>
{
    using Base = runir::kr::dl::NullaryData<runir::kr::dl::Concept<Family, runir::kr::dl::TopTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::PredicateData<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::PredicateData<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::IntersectionTag>, DlConcept<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::IntersectionTag>, DlConcept<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::UnionTag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::UnionTag>, DlConcept<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::UnionTag>, DlConcept<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::NegationTag>> :
    runir::kr::dl::UnaryData<runir::kr::dl::Concept<Family, runir::kr::dl::NegationTag>, DlConcept<Family>>
{
    using Base = runir::kr::dl::UnaryData<runir::kr::dl::Concept<Family, runir::kr::dl::NegationTag>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::ValueRestrictionTag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::ValueRestrictionTag>, DlRole<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::ValueRestrictionTag>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>> :
    runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>, DlRole<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>> :
    runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>, DlRole<Family>>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>> :
    runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>, DlRole<Family>>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>> :
    runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>, DlRole<Family>>
{
    using Base = runir::kr::dl::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
struct Data<runir::kr::dl::Concept<Family, Tag>> :
    runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::Concept<Family, Tag>, DlRole<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::QualifiedNumberRestrictionData<runir::kr::dl::Concept<Family, Tag>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
struct Data<runir::kr::dl::Concept<Family, Tag>> : runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, Tag>, DlRole<Family>, DlRole<Family>>
{
    using Base = runir::kr::dl::BinaryData<runir::kr::dl::Concept<Family, Tag>, DlRole<Family>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::RoleFillersData<runir::kr::dl::Concept<Family, runir::kr::dl::RoleFillersTag>, DlRole<Family>>
{
    using Base = runir::kr::dl::RoleFillersData<runir::kr::dl::Concept<Family, runir::kr::dl::RoleFillersTag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::OneOfTag>> : runir::kr::dl::ObjectListData<runir::kr::dl::Concept<Family, runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::ObjectListData<runir::kr::dl::Concept<Family, runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::NominalTag>> : runir::kr::dl::ObjectData<runir::kr::dl::Concept<Family, runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::ObjectData<runir::kr::dl::Concept<Family, runir::kr::dl::NominalTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                 runir::kr::dl::Register<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                              runir::kr::dl::Register<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> :
    runir::kr::dl::ReferenceData<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                 runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::ReferenceData<runir::kr::dl::Concept<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>,
                                              runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
