#ifndef RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<>
struct Data<runir::grammar::Numerical<runir::CountTag>> :
    runir::grammar::UnaryData<runir::grammar::Numerical<runir::CountTag>,
                              ::cista::offset::variant<runir::grammar::Symbol<runir::ConceptTag>, runir::grammar::Symbol<runir::RoleTag>>>
{
    using Arg = ::cista::offset::variant<runir::grammar::Symbol<runir::ConceptTag>, runir::grammar::Symbol<runir::RoleTag>>;
    using Base = runir::grammar::UnaryData<runir::grammar::Numerical<runir::CountTag>, Arg>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Numerical<runir::DistanceTag>> :
    runir::grammar::TernaryData<runir::grammar::Numerical<runir::DistanceTag>,
                                runir::grammar::Symbol<runir::ConceptTag>,
                                runir::grammar::Symbol<runir::RoleTag>,
                                runir::grammar::Symbol<runir::ConceptTag>>
{
    using Base = runir::grammar::TernaryData<runir::grammar::Numerical<runir::DistanceTag>,
                                             runir::grammar::Symbol<runir::ConceptTag>,
                                             runir::grammar::Symbol<runir::RoleTag>,
                                             runir::grammar::Symbol<runir::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
