#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/cnf_grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<>
struct Data<runir::cnf_grammar::Numerical<runir::CountTag>>
{
    using Arg =
        ::cista::offset::variant<tyr::Index<runir::cnf_grammar::NonTerminal<runir::ConceptTag>>, tyr::Index<runir::cnf_grammar::NonTerminal<runir::RoleTag>>>;

    Index<runir::cnf_grammar::Numerical<runir::CountTag>> index;
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

template<>
struct Data<runir::cnf_grammar::Numerical<runir::DistanceTag>> :
    runir::cnf_grammar::TernaryData<runir::cnf_grammar::Numerical<runir::DistanceTag>,
                                    runir::cnf_grammar::NonTerminal<runir::ConceptTag>,
                                    runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                    runir::cnf_grammar::NonTerminal<runir::ConceptTag>>
{
    using Base = runir::cnf_grammar::TernaryData<runir::cnf_grammar::Numerical<runir::DistanceTag>,
                                                 runir::cnf_grammar::NonTerminal<runir::ConceptTag>,
                                                 runir::cnf_grammar::NonTerminal<runir::RoleTag>,
                                                 runir::cnf_grammar::NonTerminal<runir::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
