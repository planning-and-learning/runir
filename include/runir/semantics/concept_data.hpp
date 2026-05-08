#ifndef RUNIR_SEMANTICS_CONCEPT_DATA_HPP_
#define RUNIR_SEMANTICS_CONCEPT_DATA_HPP_

#include "runir/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Data<runir::Concept<runir::BotTag>> : runir::semantics::NullaryData<runir::Concept<runir::BotTag>>
{
    using Base = runir::semantics::NullaryData<runir::Concept<runir::BotTag>>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::TopTag>> : runir::semantics::NullaryData<runir::Concept<runir::TopTag>>
{
    using Base = runir::semantics::NullaryData<runir::Concept<runir::TopTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::Concept<runir::AtomicStateTag<T>>> : runir::semantics::PredicateData<runir::Concept<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::semantics::PredicateData<runir::Concept<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::Concept<runir::AtomicGoalTag<T>>> : runir::semantics::PredicateData<runir::Concept<runir::AtomicGoalTag<T>>, T>
{
    using Base = runir::semantics::PredicateData<runir::Concept<runir::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::IntersectionTag>> :
    runir::semantics::BinaryData<runir::Concept<runir::IntersectionTag>, runir::Constructor<runir::ConceptTag>, runir::Constructor<runir::ConceptTag>>
{
    using Base =
        runir::semantics::BinaryData<runir::Concept<runir::IntersectionTag>, runir::Constructor<runir::ConceptTag>, runir::Constructor<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::UnionTag>> :
    runir::semantics::BinaryData<runir::Concept<runir::UnionTag>, runir::Constructor<runir::ConceptTag>, runir::Constructor<runir::ConceptTag>>
{
    using Base = runir::semantics::BinaryData<runir::Concept<runir::UnionTag>, runir::Constructor<runir::ConceptTag>, runir::Constructor<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::NegationTag>> : runir::semantics::UnaryData<runir::Concept<runir::NegationTag>, runir::Constructor<runir::ConceptTag>>
{
    using Base = runir::semantics::UnaryData<runir::Concept<runir::NegationTag>, runir::Constructor<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::ValueRestrictionTag>> :
    runir::semantics::BinaryData<runir::Concept<runir::ValueRestrictionTag>, runir::Constructor<runir::RoleTag>, runir::Constructor<runir::ConceptTag>>
{
    using Base =
        runir::semantics::BinaryData<runir::Concept<runir::ValueRestrictionTag>, runir::Constructor<runir::RoleTag>, runir::Constructor<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::ExistentialQuantificationTag>> :
    runir::semantics::BinaryData<runir::Concept<runir::ExistentialQuantificationTag>, runir::Constructor<runir::RoleTag>, runir::Constructor<runir::ConceptTag>>
{
    using Base = runir::semantics::
        BinaryData<runir::Concept<runir::ExistentialQuantificationTag>, runir::Constructor<runir::RoleTag>, runir::Constructor<runir::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::RoleValueMapContainmentTag>> :
    runir::semantics::BinaryData<runir::Concept<runir::RoleValueMapContainmentTag>, runir::Constructor<runir::RoleTag>, runir::Constructor<runir::RoleTag>>
{
    using Base =
        runir::semantics::BinaryData<runir::Concept<runir::RoleValueMapContainmentTag>, runir::Constructor<runir::RoleTag>, runir::Constructor<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::RoleValueMapEqualityTag>> :
    runir::semantics::BinaryData<runir::Concept<runir::RoleValueMapEqualityTag>, runir::Constructor<runir::RoleTag>, runir::Constructor<runir::RoleTag>>
{
    using Base =
        runir::semantics::BinaryData<runir::Concept<runir::RoleValueMapEqualityTag>, runir::Constructor<runir::RoleTag>, runir::Constructor<runir::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::Concept<runir::NominalTag>> : runir::semantics::ObjectData<runir::Concept<runir::NominalTag>>
{
    using Base = runir::semantics::ObjectData<runir::Concept<runir::NominalTag>>;
    using Base::Base;
};

}

#endif
