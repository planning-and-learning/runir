#ifndef RUNIR_KR_PS_EXT_ACTION_RULE_CONTRACT_ERROR_HPP_
#define RUNIR_KR_PS_EXT_ACTION_RULE_CONTRACT_ERROR_HPP_

#include <stdexcept>

namespace runir::kr::ps::ext
{

class ActionRuleContractError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

}  // namespace runir::kr::ps::ext

#endif
