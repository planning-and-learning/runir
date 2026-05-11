#ifndef RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<tyr::formalism::FactKind T>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>> :
    runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>
{
    using Base = runir::kr::dl::cnf_grammar::PredicateData<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::AtomicStateTag<T>>, T>;
    using Base::Base;
};

template<>
struct Data<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NonemptyTag>>
{
    using Arg = ::cista::offset::variant<tyr::Index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>,
                                         tyr::Index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>>;

    Index<runir::kr::dl::cnf_grammar::Boolean<runir::kr::dl::NonemptyTag>> index;
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
