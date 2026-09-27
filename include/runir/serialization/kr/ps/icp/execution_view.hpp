#ifndef RUNIR_SERIALIZATION_KR_PS_ICP_EXECUTION_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_ICP_EXECUTION_VIEW_HPP_

#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/serialization/kr/dl/semantics/denotation_view.hpp"
#include "runir/serialization/kr/dl/semantics/register_values_view.hpp"
#include "runir/serialization/kr/ps/icp/program_view.hpp"

#include <tyr/planning/state_repository.hpp>
#include <tyr/serialization/planning/state_view.hpp>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::icp::Histories>, C>>)
{
    ar.field("concepts", [](const auto& value) { return value.get_concepts(); });
}

template<typename Archive, tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<runir::kr::ps::icp::ProgramStateView<Kind>>)
{
    ar.field("program", [](const auto& value) { return value.get_program(); });
    ar.field("memory_state", [](const auto& value) { return value.get_memory_state(); });
    ar.field("registers", [](const auto& value) { return value.get_registers(); });
    ar.field("histories", [](const auto& value) { return value.get_histories(); });
    ar.field("state", [](const auto& value) { return value.get_state(); });
}

}

#endif
