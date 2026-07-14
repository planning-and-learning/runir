#ifndef RUNIR_KR_DL_ROLE_DATA_HPP_
#define RUNIR_KR_DL_ROLE_DATA_HPP_

#include "runir/kr/dl/concept_data.hpp"
#include "runir/kr/dl/semantics/data_helpers.hpp"

#include <concepts>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Role<Family, runir::kr::dl::UniversalTag>> :
    runir::kr::dl::semantics::NullaryData<runir::kr::dl::Role<Family, runir::kr::dl::UniversalTag>>
{
    using Base = runir::kr::dl::semantics::NullaryData<runir::kr::dl::Role<Family, runir::kr::dl::UniversalTag>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Role<Family, runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Role<Family, runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Role<Family, runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Role<Family, runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Role<Family, runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Role<Family, runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
             || std::same_as<Tag, runir::kr::dl::CompositionTag>)
struct Data<runir::kr::dl::Role<Family, Tag>> : runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<Family, Tag>, DlRole<Family>, DlRole<Family>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<Family, Tag>, DlRole<Family>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires(std::same_as<Tag, runir::kr::dl::ComplementTag> || std::same_as<Tag, runir::kr::dl::InverseTag>
             || std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
struct Data<runir::kr::dl::Role<Family, Tag>> : runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<Family, Tag>, DlRole<Family>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<Family, Tag>, DlRole<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Role<Family, runir::kr::dl::RestrictionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<Family, runir::kr::dl::RestrictionTag>, DlRole<Family>, DlConcept<Family>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<Family, runir::kr::dl::RestrictionTag>, DlRole<Family>, DlConcept<Family>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::Role<Family, runir::kr::dl::IdentityTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<Family, runir::kr::dl::IdentityTag>, DlConcept<Family>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<Family, runir::kr::dl::IdentityTag>, DlConcept<Family>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>> :
    runir::kr::dl::semantics::ReferenceData<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                            runir::kr::dl::Register<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::ReferenceData<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::RegisterTag>,
                                                         runir::kr::dl::Register<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>> :
    runir::kr::dl::semantics::ReferenceData<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>,
                                            runir::kr::dl::Argument<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::ReferenceData<runir::kr::dl::Role<runir::kr::ExtFamilyTag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>,
                                                         runir::kr::dl::Argument<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
