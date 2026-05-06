#ifndef RUNIR_SEMANTICS_ROLE_DATA_HPP_
#define RUNIR_SEMANTICS_ROLE_DATA_HPP_

#include "runir/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>

namespace tyr
{

template<>
struct Data<runir::Role<runir::UniversalTag>> : runir::semantics::NullaryData<runir::Role<runir::UniversalTag>>
{
    using Base = runir::semantics::NullaryData<runir::Role<runir::UniversalTag>>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::Role<runir::AtomicStateTag<T>>> : runir::semantics::PredicateData<runir::Role<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::semantics::PredicateData<runir::Role<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<tyr::formalism::FactKind T>
struct Data<runir::Role<runir::AtomicGoalTag<T>>> : runir::semantics::PredicateData<runir::Role<runir::AtomicGoalTag<T>>, T>
{
    using Base = runir::semantics::PredicateData<runir::Role<runir::AtomicGoalTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::IntersectionTag>> :
    runir::semantics::BinaryData<runir::Role<runir::IntersectionTag>, Index<runir::Constructor<runir::RoleTag>>, Index<runir::Constructor<runir::RoleTag>>>
{
    using Base =
        runir::semantics::BinaryData<runir::Role<runir::IntersectionTag>, Index<runir::Constructor<runir::RoleTag>>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::UnionTag>> :
    runir::semantics::BinaryData<runir::Role<runir::UnionTag>, Index<runir::Constructor<runir::RoleTag>>, Index<runir::Constructor<runir::RoleTag>>>
{
    using Base =
        runir::semantics::BinaryData<runir::Role<runir::UnionTag>, Index<runir::Constructor<runir::RoleTag>>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::ComplementTag>> : runir::semantics::UnaryData<runir::Role<runir::ComplementTag>, Index<runir::Constructor<runir::RoleTag>>>
{
    using Base = runir::semantics::UnaryData<runir::Role<runir::ComplementTag>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::InverseTag>> : runir::semantics::UnaryData<runir::Role<runir::InverseTag>, Index<runir::Constructor<runir::RoleTag>>>
{
    using Base = runir::semantics::UnaryData<runir::Role<runir::InverseTag>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::CompositionTag>> :
    runir::semantics::BinaryData<runir::Role<runir::CompositionTag>, Index<runir::Constructor<runir::RoleTag>>, Index<runir::Constructor<runir::RoleTag>>>
{
    using Base =
        runir::semantics::BinaryData<runir::Role<runir::CompositionTag>, Index<runir::Constructor<runir::RoleTag>>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::TransitiveClosureTag>> :
    runir::semantics::UnaryData<runir::Role<runir::TransitiveClosureTag>, Index<runir::Constructor<runir::RoleTag>>>
{
    using Base = runir::semantics::UnaryData<runir::Role<runir::TransitiveClosureTag>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::ReflexiveTransitiveClosureTag>> :
    runir::semantics::UnaryData<runir::Role<runir::ReflexiveTransitiveClosureTag>, Index<runir::Constructor<runir::RoleTag>>>
{
    using Base = runir::semantics::UnaryData<runir::Role<runir::ReflexiveTransitiveClosureTag>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::RestrictionTag>> :
    runir::semantics::BinaryData<runir::Role<runir::RestrictionTag>, Index<runir::Constructor<runir::RoleTag>>, Index<runir::Constructor<runir::ConceptTag>>>
{
    using Base = runir::semantics::
        BinaryData<runir::Role<runir::RestrictionTag>, Index<runir::Constructor<runir::RoleTag>>, Index<runir::Constructor<runir::ConceptTag>>>;
    using Base::Base;
};

template<>
struct Data<runir::Role<runir::IdentityTag>> : runir::semantics::UnaryData<runir::Role<runir::IdentityTag>, Index<runir::Constructor<runir::ConceptTag>>>
{
    using Base = runir::semantics::UnaryData<runir::Role<runir::IdentityTag>, Index<runir::Constructor<runir::ConceptTag>>>;
    using Base::Base;
};

}

#endif
