#ifndef RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<tyr::formalism::FactKind T>
struct Data<runir::grammar::Boolean<runir::AtomicStateTag<T>>> : runir::grammar::PredicateData<runir::grammar::Boolean<runir::AtomicStateTag<T>>, T>
{
    using Base = runir::grammar::PredicateData<runir::grammar::Boolean<runir::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::grammar::Boolean<runir::NonemptyTag>> :
    runir::grammar::UnaryData<runir::grammar::Boolean<runir::NonemptyTag>,
                              ::cista::offset::variant<runir::grammar::Symbol<runir::ConceptTag>, runir::grammar::Symbol<runir::RoleTag>>>
{
    using Arg = ::cista::offset::variant<runir::grammar::Symbol<runir::ConceptTag>, runir::grammar::Symbol<runir::RoleTag>>;
    using Base = runir::grammar::UnaryData<runir::grammar::Boolean<runir::NonemptyTag>, Arg>;
    using Base::Base;
};

}  // namespace tyr

#endif
