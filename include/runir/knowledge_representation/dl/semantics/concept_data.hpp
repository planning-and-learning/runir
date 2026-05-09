#ifndef RUNIR_SEMANTICS_CONCEPT_DATA_HPP_
#define RUNIR_SEMANTICS_CONCEPT_DATA_HPP_

#include "runir/knowledge_representation/dl/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::BotTag>> : runir::kr::dl::semantics::NullaryData<runir::kr::dl::Concept<runir::kr::dl::BotTag>>
{
    using Base = runir::kr::dl::semantics::NullaryData<runir::kr::dl::Concept<runir::kr::dl::BotTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::TopTag>> : runir::kr::dl::semantics::NullaryData<runir::kr::dl::Concept<runir::kr::dl::TopTag>>
{
    using Base = runir::kr::dl::semantics::NullaryData<runir::kr::dl::Concept<runir::kr::dl::TopTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Concept<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Concept<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Concept<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Concept<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Concept<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::IntersectionTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::IntersectionTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::UnionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::UnionTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::UnionTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::NegationTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::Concept<runir::kr::dl::NegationTag>, runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::Concept<runir::kr::dl::NegationTag>, runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::ValueRestrictionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::ValueRestrictionTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::ValueRestrictionTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::ExistentialQuantificationTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::ExistentialQuantificationTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::ExistentialQuantificationTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::RoleValueMapContainmentTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::RoleValueMapContainmentTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::RoleValueMapContainmentTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::RoleValueMapEqualityTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::RoleValueMapEqualityTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Concept<runir::kr::dl::RoleValueMapEqualityTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Concept<runir::kr::dl::NominalTag>> : runir::kr::dl::semantics::ObjectData<runir::kr::dl::Concept<runir::kr::dl::NominalTag>>
{
    using Base = runir::kr::dl::semantics::ObjectData<runir::kr::dl::Concept<runir::kr::dl::NominalTag>>;
    using Base::Base;
};

}

#endif
