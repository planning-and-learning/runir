#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

inline void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                       ::runir::kr::ps::ext::ExecutionPhase value, Dictionaries*)
{
    result = std::string(::runir::kr::ps::ext::to_string(value));
}

}

#endif
