#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/cnf_grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<>
struct Data<runir::cnf_grammar::Numerical<runir::CountTag>> :
    runir::cnf_grammar::UnaryData<runir::cnf_grammar::Numerical<runir::CountTag>,
                                  ::cista::offset::variant<runir::cnf_grammar::Symbol<runir::ConceptTag>, runir::cnf_grammar::Symbol<runir::RoleTag>>>
{
    using Arg = ::cista::offset::variant<runir::cnf_grammar::Symbol<runir::ConceptTag>, runir::cnf_grammar::Symbol<runir::RoleTag>>;
    using Base = runir::cnf_grammar::UnaryData<runir::cnf_grammar::Numerical<runir::CountTag>, Arg>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Numerical<runir::DistanceTag>> :
    runir::cnf_grammar::TernaryData<runir::cnf_grammar::Numerical<runir::DistanceTag>,
                                    runir::cnf_grammar::Symbol<runir::ConceptTag>,
                                    runir::cnf_grammar::Symbol<runir::RoleTag>,
                                    runir::cnf_grammar::Symbol<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::TernaryData<runir::cnf_grammar::Numerical<runir::DistanceTag>,
                                                 runir::cnf_grammar::Symbol<runir::ConceptTag>,
                                                 runir::cnf_grammar::Symbol<runir::RoleTag>,
                                                 runir::cnf_grammar::Symbol<runir::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
