#ifndef RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/cnf_grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<tyr::formalism::FactKind T>
struct Data<runir::cnf_grammar::Boolean<runir::AtomicStateTag<T>>> : runir::cnf_grammar::PredicateData<runir::cnf_grammar::Boolean<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::cnf_grammar::PredicateData<runir::cnf_grammar::Boolean<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::cnf_grammar::Boolean<runir::NonemptyTag>> :
    runir::cnf_grammar::UnaryData<runir::cnf_grammar::Boolean<runir::NonemptyTag>,
                                  ::cista::offset::variant<runir::cnf_grammar::Symbol<runir::ConceptTag>, runir::cnf_grammar::Symbol<runir::RoleTag>>>
{
    using Arg = ::cista::offset::variant<runir::cnf_grammar::Symbol<runir::ConceptTag>, runir::cnf_grammar::Symbol<runir::RoleTag>>;
    using Base = runir::cnf_grammar::UnaryData<runir::cnf_grammar::Boolean<runir::NonemptyTag>, Arg>;
    using Base::Base;
};

}  // namespace tyr

#endif
