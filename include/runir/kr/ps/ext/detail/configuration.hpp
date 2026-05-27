#ifndef RUNIR_KR_PS_EXT_DETAIL_CONFIGURATION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_CONFIGURATION_HPP_

#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/memory_state.hpp"

#include <sstream>
#include <string>
#include <unordered_set>
#include <variant>

namespace runir::kr::ps::ext::detail
{

inline const char* module_program_memory_state_kind_key(const MemoryStateVariant& memory_state) noexcept
{
    return std::holds_alternative<InternalMemoryState>(memory_state) ? "internal" : "external";
}

template<tyr::planning::TaskKind Kind>
std::string module_program_configuration_key(const EvaluationContext<Kind>& context)
{
    auto os = std::ostringstream {};
    os << tyr::uint_t(context.get_state().get_index()) << '|' << tyr::uint_t(context.get_module().get_index()) << '|'
       << tyr::uint_t(context.get_memory_state().get_index());
    return os.str();
}

template<tyr::planning::TaskKind Kind>
std::string module_program_configuration_key(const EvaluationContext<Kind>& context, const MemoryStateVariant& memory_state)
{
    return std::string(module_program_memory_state_kind_key(memory_state)) + "|" + module_program_configuration_key(context);
}

template<tyr::planning::TaskKind Kind>
class ModuleProgramConfigurationHistory
{
private:
    std::unordered_set<std::string> m_seen;

public:
    bool remember(const EvaluationContext<Kind>& context, const MemoryStateVariant& memory_state)
    {
        return m_seen.insert(module_program_configuration_key(context, memory_state)).second;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
