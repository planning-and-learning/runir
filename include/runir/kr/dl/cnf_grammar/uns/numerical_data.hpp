#ifndef RUNIR_KR_DL_CNF_GRAMMAR_UNS_NUMERICAL_DATA_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_UNS_NUMERICAL_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/numerical_data.hpp"
#include "runir/kr/dl/uns/declarations.hpp"

namespace ygg
{

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>> :
    runir::kr::dl::semantics::IdentifierData<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>
{
    using Base = runir::kr::dl::semantics::IdentifierData<runir::kr::dl::cnf_grammar::Numerical<Family, runir::kr::dl::NumericalConstantTag>, ygg::uint_t>;
    using Base::Base;
};

// Binary numerical operators over two numerical non-terminals (CNF form).
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::NumericalBinaryTag Tag>
struct Data<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Numerical<Family, Tag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::NumericalTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
