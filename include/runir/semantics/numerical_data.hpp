#ifndef RUNIR_SEMANTICS_NUMERICAL_DATA_HPP_
#define RUNIR_SEMANTICS_NUMERICAL_DATA_HPP_

#include "runir/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<>
struct Data<runir::Numerical<runir::CountTag>> :
    runir::semantics::UnaryData<runir::Numerical<runir::CountTag>,
                                ::cista::offset::variant<Index<runir::Constructor<runir::ConceptTag>>, Index<runir::Constructor<runir::RoleTag>>>>
{
    using ConstructorVariant = ::cista::offset::variant<Index<runir::Constructor<runir::ConceptTag>>, Index<runir::Constructor<runir::RoleTag>>>;
    using Base = runir::semantics::UnaryData<runir::Numerical<runir::CountTag>, ConstructorVariant>;
    using Base::Base;
};

template<>
struct Data<runir::Numerical<runir::DistanceTag>> :
    runir::semantics::TernaryData<runir::Numerical<runir::DistanceTag>,
                                  Index<runir::Constructor<runir::ConceptTag>>,
                                  Index<runir::Constructor<runir::RoleTag>>,
                                  Index<runir::Constructor<runir::ConceptTag>>>
{
    using Base = runir::semantics::TernaryData<runir::Numerical<runir::DistanceTag>,
                                               Index<runir::Constructor<runir::ConceptTag>>,
                                               Index<runir::Constructor<runir::RoleTag>>,
                                               Index<runir::Constructor<runir::ConceptTag>>>;
    using Base::Base;
};

static_assert(!uses_trivial_storage_v<runir::Numerical<runir::CountTag>>);

}

#endif
