#ifndef RUNIR_KR_PS_EXT_DETAIL_CONFIGURATION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_CONFIGURATION_HPP_

#include "runir/kr/ps/ext/evaluation_context.hpp"

#include <sstream>
#include <string>
#include <unordered_set>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
std::string module_program_configuration_key(const EvaluationContext<Kind>& context)
{
    auto os = std::ostringstream {};
    os << tyr::uint_t(context.get_state().get_index()) << '|' << tyr::uint_t(context.get_module().get_index()) << '|'
       << tyr::uint_t(context.get_memory_state().get_index());
    return os.str();
}

template<tyr::planning::TaskKind Kind>
class ModuleProgramConfigurationHistory
{
private:
    std::unordered_set<std::string> m_seen;

public:
    bool remember(const EvaluationContext<Kind>& context) { return m_seen.insert(module_program_configuration_key(context)).second; }
};

}  // namespace runir::kr::ps::ext::detail

#endif
