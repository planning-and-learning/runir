#ifndef RUNIR_KR_PS_ICP_MODULE_DATA_HPP_
#define RUNIR_KR_PS_ICP_MODULE_DATA_HPP_

#include "runir/kr/dl/argument_index.hpp"
#include "runir/kr/dl/register_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/icp/memory_state_index.hpp"
#include "runir/kr/ps/icp/module_index.hpp"
#include "runir/kr/ps/icp/module_symbol_index.hpp"
#include "runir/kr/ps/icp/object_reference.hpp"
#include "runir/kr/ps/icp/rule_data.hpp"
#include "runir/kr/ps/icp/rule_index.hpp"
#include "runir/kr/ps/icp/rule_variant_index.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::icp::Module>
{
    Index<runir::kr::ps::icp::Module> index;
    Index<runir::kr::ps::icp::ModuleSymbol> symbol;
    IndexList<runir::kr::dl::Register<runir::kr::dl::ConceptTag>> concept_registers;
    IndexList<runir::kr::dl::Register<runir::kr::dl::RoleTag>> role_registers;
    IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> concept_features;
    IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>> role_features;
    IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::BooleanFeature>> boolean_features;
    IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::NumericalFeature>> numerical_features;
    IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>> query_features;
    Index<runir::kr::ps::icp::MemoryState> entry_memory_state;
    IndexList<runir::kr::ps::icp::MemoryState> memory_states;
    IndexMatrix<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>> memory_transitions;
    ::cista::offset::vector<runir::kr::ps::icp::ResetPair> reset_pairs;

    Data() = default;
    Data(Index<runir::kr::ps::icp::ModuleSymbol> symbol_) : index(), symbol(symbol_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(symbol);
        ygg::clear(concept_registers);
        ygg::clear(role_registers);
        ygg::clear(concept_features);
        ygg::clear(role_features);
        ygg::clear(boolean_features);
        ygg::clear(numerical_features);
        ygg::clear(query_features);
        ygg::clear(entry_memory_state);
        ygg::clear(memory_states);
        ygg::clear(memory_transitions);
        ygg::clear(reset_pairs);
    }

    auto cista_members() const noexcept
    {
        return std::tie(index,
                        symbol,
                        concept_registers,
                        role_registers,
                        concept_features,
                        role_features,
                        boolean_features,
                        numerical_features,
                        query_features,
                        entry_memory_state,
                        memory_states,
                        memory_transitions,
                        reset_pairs);
    }
    auto identifying_members() const noexcept
    {
        return std::tie(symbol,
                        concept_registers,
                        role_registers,
                        concept_features,
                        role_features,
                        boolean_features,
                        numerical_features,
                        query_features,
                        entry_memory_state,
                        memory_states,
                        memory_transitions,
                        reset_pairs);
    }
};

}  // namespace ygg

#endif
