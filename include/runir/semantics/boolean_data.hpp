#ifndef RUNIR_SEMANTICS_BOOLEAN_DATA_HPP_
#define RUNIR_SEMANTICS_BOOLEAN_DATA_HPP_

#include "runir/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<tyr::formalism::FactKind T>
struct Data<runir::Boolean<runir::AtomicStateTag<T>>> : runir::semantics::PredicateData<runir::Boolean<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::semantics::PredicateData<runir::Boolean<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::Boolean<runir::NonemptyTag>> :
    runir::semantics::UnaryData<runir::Boolean<runir::NonemptyTag>,
                                ::cista::offset::variant<Index<runir::Constructor<runir::ConceptTag>>, Index<runir::Constructor<runir::RoleTag>>>>
{
    using ConstructorVariant = ::cista::offset::variant<Index<runir::Constructor<runir::ConceptTag>>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base = runir::semantics::UnaryData<runir::Boolean<runir::NonemptyTag>, ConstructorVariant>;
    using Base::Base;
};

static_assert(!uses_trivial_storage_v<runir::Boolean<runir::NonemptyTag>>);

}

#endif
