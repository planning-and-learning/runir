#ifndef RUNIR_KR_DL_CNF_GRAMMAR_UNS_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_UNS_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/cnf_grammar/boolean_data.hpp"
#include "runir/kr/dl/uns/declarations.hpp"

namespace ygg
{

// Comparison operators over two operand-category non-terminals (CNF form).
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::ComparisonTag Tag>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::comparison_operand_t<Tag>>>;
    using Base::Base;
};

template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>> :
    runir::kr::dl::semantics::IdentifierData<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>
{
    using Base = runir::kr::dl::semantics::IdentifierData<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::BooleanConstantTag>, bool>;
    using Base::Base;
};

// Binary logical operators over two boolean non-terminals (CNF form).
template<runir::kr::dl::FamilyTag Family, runir::kr::dl::LogicalBinaryTag Tag>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>> :
    runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>,
                                           runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::cnf_grammar::BinaryData<runir::kr::dl::cnf_grammar::Boolean<Family, Tag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>,
                                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

// Unary logical operator (not) over one boolean non-terminal (CNF form).
template<runir::kr::dl::FamilyTag Family>
struct Data<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::NotTag>> :
    runir::kr::dl::semantics::UnaryData<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::NotTag>,
                                        runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>
{
    using Base = runir::kr::dl::semantics::UnaryData<runir::kr::dl::cnf_grammar::Boolean<Family, runir::kr::dl::NotTag>,
                                                     runir::kr::dl::cnf_grammar::NonTerminal<Family, runir::kr::dl::BooleanTag>>;
    using Base::Base;
};

}  // namespace ygg

#endif
