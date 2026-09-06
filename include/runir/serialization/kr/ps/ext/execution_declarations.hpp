#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_EXECUTION_DECLARATIONS_HPP_

#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

#include <type_traits>

namespace ygg::serialization
{

inline void tag_invoke(boost::json::value_from_tag, boost::json::value& result,
                       ::runir::kr::ps::ext::ExecutionPhase value, Dictionaries* dictionaries)
{
    using ::runir::kr::ps::ext::ExecutionPhase;
    const auto id = static_cast<std::underlying_type_t<ExecutionPhase>>(value);
    switch (value)
    {
        case ExecutionPhase::INTERNAL:
            result = dictionaries->add_kind("ExecutionPhase", id, "INTERNAL");
            return;
        case ExecutionPhase::EXTERNAL:
            result = dictionaries->add_kind("ExecutionPhase", id, "EXTERNAL");
            return;
    }
    throw std::invalid_argument("invalid ExecutionPhase");
}

}

#endif
