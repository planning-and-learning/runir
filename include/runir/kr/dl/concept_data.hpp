#ifndef RUNIR_KR_DL_CONCEPT_DATA_HPP_
#define RUNIR_KR_DL_CONCEPT_DATA_HPP_

#include "runir/kr/dl/indices.hpp"
#include "runir/kr/dl/semantics/data_helpers.hpp"

#include <concepts>
#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::kr::dl::FamilyTag Family>
using DlConcept = runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>;

template<runir::kr::dl::FamilyTag Family>
using DlRole = runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>;

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::BotTag>> :
    runir::kr::dl::semantics::NullaryData<runir::kr::dl::Concept<Family, runir::kr::dl::BotTag>>
{
    using Base = runir::kr::dl::semantics::NullaryData<runir::kr::dl::Concept<Family, runir::kr::dl::BotTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::TopTag>> :
    runir::kr::dl::semantics::NullaryData<runir::kr::dl::Concept<Family, runir::kr::dl::TopTag>>
{
    using Base = runir::kr::dl::semantics::NullaryData<runir::kr::dl::Concept<Family, runir::kr::dl::TopTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Concept<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::IntersectionTag>, DlConcept<Family>, DlConcept<Family>>
{
    using Base =
        runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::IntersectionTag>, DlConcept<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::UnionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::UnionTag>, DlConcept<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::UnionTag>, DlConcept<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::NegationTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::Concept<Family, runir::kr::dl::NegationTag>, DlConcept<Family>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::Concept<Family, runir::kr::dl::NegationTag>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::ValueRestrictionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::ValueRestrictionTag>, DlRole<Family>, DlConcept<Family>>
{
    using Base =
        runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::ValueRestrictionTag>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>, DlRole<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::semantics::
        BinaryData<runir::kr::dl::Concept<Family, runir::kr::dl::ExistentialQuantificationTag>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>> :
    runir::kr::dl::semantics::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>, DlRole<Family>>
{
    using Base =
        runir::kr::dl::semantics::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::AtLeastNumberRestrictionTag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>> :
    runir::kr::dl::semantics::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>, DlRole<Family>>
{
    using Base =
        runir::kr::dl::semantics::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::AtMostNumberRestrictionTag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>> :
    runir::kr::dl::semantics::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>, DlRole<Family>>
{
    using Base =
        runir::kr::dl::semantics::NumberRestrictionData<runir::kr::dl::Concept<Family, runir::kr::dl::ExactNumberRestrictionTag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::QualifiedAtLeastNumberRestrictionTag> || std::same_as<Tag, runir::kr::dl::QualifiedAtMostNumberRestrictionTag>
             || std::same_as<Tag, runir::kr::dl::QualifiedExactNumberRestrictionTag>)
struct Data<runir::kr::dl::Concept<Family, Tag>> :
    runir::kr::dl::semantics::QualifiedNumberRestrictionData<runir::kr::dl::Concept<Family, Tag>, DlRole<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::semantics::QualifiedNumberRestrictionData<runir::kr::dl::Concept<Family, Tag>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::RoleValueMapTag> || std::same_as<Tag, runir::kr::dl::AgreementTag>)
struct Data<runir::kr::dl::Concept<Family, Tag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, Tag>, DlRole<Family>, DlRole<Family>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<Family, Tag>, DlRole<Family>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::RoleFillersTag>> :
    runir::kr::dl::semantics::RoleFillersData<runir::kr::dl::Concept<Family, runir::kr::dl::RoleFillersTag>, DlRole<Family>>
{
    using Base = runir::kr::dl::semantics::RoleFillersData<runir::kr::dl::Concept<Family, runir::kr::dl::RoleFillersTag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::OneOfTag>> :
    runir::kr::dl::semantics::ObjectListData<runir::kr::dl::Concept<Family, runir::kr::dl::OneOfTag>>
{
    using Base = runir::kr::dl::semantics::ObjectListData<runir::kr::dl::Concept<Family, runir::kr::dl::OneOfTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Concept<Family, runir::kr::dl::NominalTag>> :
    runir::kr::dl::semantics::ObjectData<runir::kr::dl::Concept<Family, runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::semantics::ObjectData<runir::kr::dl::Concept<Family, runir::kr::dl::NominalTag>>;
    using Base::Base;
};


}  // namespace tyr

#endif
