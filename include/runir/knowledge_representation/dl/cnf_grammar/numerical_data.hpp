#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/knowledge_representation/dl/cnf_grammar/data_helpers.hpp"

#include <tyr/common/variant.hpp>

namespace tyr
{

template<>
struct Data<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::CountTag>>
{
    using Arg = ::cista::offset::variant<tyr::Index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>,
                                         tyr::Index<runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>>>;

    Index<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::CountTag>> index;
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
struct Data<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::DistanceTag>> :
    runir::kr::dl::cnf_grammar::TernaryData<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::DistanceTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::TernaryData<runir::kr::dl::cnf_grammar::Numerical<runir::kr::dl::DistanceTag>,
                                                         runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>,
                                                         runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::RoleTag>,
                                                         runir::kr::dl::cnf_grammar::NonTerminal<runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace tyr

#endif
