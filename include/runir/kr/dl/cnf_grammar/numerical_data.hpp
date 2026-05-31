#ifndef RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_
#define RUNIR_CNF_GRAMMAR_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/data_helpers.hpp"

#include <yggdrasil/containers/variant.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::CountTag>>
{
    using Arg = ::cista::offset::variant<ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>,
                                         ygg::Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>>>;

    Index<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::CountTag>> index;
    Arg arg;

    Data() = default;
    explicit Data(Arg arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::DistanceTag>> :
    runir::kr::dl::cnf_grammar::TernaryData<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::DistanceTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                            runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>
{
    using Base = runir::kr::dl::cnf_grammar::TernaryData<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::DistanceTag>,
                                                         runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>,
                                                         runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::RoleTag>,
                                                         runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::ConceptTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
