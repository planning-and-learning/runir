#ifndef RUNIR_SEMANTICS_ROLE_DATA_HPP_
#define RUNIR_SEMANTICS_ROLE_DATA_HPP_

#include "runir/knowledge_representation/dl/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::UniversalTag>> : runir::kr::dl::semantics::NullaryData<runir::kr::dl::Role<runir::kr::dl::UniversalTag>>
{
    using Base = runir::kr::dl::semantics::NullaryData<runir::kr::dl::Role<runir::kr::dl::UniversalTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Role<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Role<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Role<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::Role<runir::kr::dl::AtomicGoalTag<T>>> :
    runir::kr::dl::semantics::PredicateData<runir::kr::dl::Role<runir::kr::dl::AtomicGoalTag<T>>, T>
{
    using Base = runir::kr::dl::semantics::PredicateData<runir::kr::dl::Role<runir::kr::dl::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::IntersectionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<runir::kr::dl::IntersectionTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<runir::kr::dl::IntersectionTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::UnionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<runir::kr::dl::UnionTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<runir::kr::dl::UnionTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::ComplementTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::ComplementTag>, runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::ComplementTag>, runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::InverseTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::InverseTag>, runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::InverseTag>, runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::CompositionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<runir::kr::dl::CompositionTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<runir::kr::dl::CompositionTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::TransitiveClosureTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::TransitiveClosureTag>, runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base =
        runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::TransitiveClosureTag>, runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>, runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::ReflexiveTransitiveClosureTag>,
                                                     runir::kr::dl::Constructor<runir::kr::dl::RoleTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::RestrictionTag>> :
    runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<runir::kr::dl::RestrictionTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                         runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::BinaryData<runir::kr::dl::Role<runir::kr::dl::RestrictionTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::RoleTag>,
                                                      runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::Role<runir::kr::dl::IdentityTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::IdentityTag>, runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::Role<runir::kr::dl::IdentityTag>, runir::kr::dl::Constructor<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}

#endif
