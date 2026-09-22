#ifndef RUNIR_KR_PS_EXT_ACTION_RULE_CONTRACT_ERROR_HPP_
#define RUNIR_KR_PS_EXT_ACTION_RULE_CONTRACT_ERROR_HPP_

#include <stdexcept>

#if defined(__clang__) || defined(__GNUC__)
#define RUNIR_KR_PS_EXT_ERROR_API __attribute__((visibility("default")))
#else
#define RUNIR_KR_PS_EXT_ERROR_API
#endif

namespace runir::kr::ps::ext
{

// The native library and Python exception translator must share this type's RTTI.
class RUNIR_KR_PS_EXT_ERROR_API ActionRuleContractError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

}  // namespace runir::kr::ps::ext

#undef RUNIR_KR_PS_EXT_ERROR_API

#endif
