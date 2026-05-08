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
struct Data<runir::cnf_grammar::Boolean<runir::NonemptyTag>>
{
    using Arg =
        ::cista::offset::variant<tyr::Index<runir::cnf_grammar::NonTerminal<runir::ConceptTag>>, tyr::Index<runir::cnf_grammar::NonTerminal<runir::RoleTag>>>;

    Index<runir::cnf_grammar::Boolean<runir::NonemptyTag>> index;
    Arg arg;

    Data() = default;
    explicit Data(Arg arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

}  // namespace tyr

#endif
