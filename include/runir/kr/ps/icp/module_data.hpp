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

#include <cista/containers/vector.h>
#include <tuple>
#include <utility>
#include <vector>
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
    // Registers live in the description-logic repository, hence the separate context parameter.
    Data(Index<runir::kr::ps::icp::ModuleSymbol> symbol_,
         IndexList<runir::kr::dl::Register<runir::kr::dl::ConceptTag>> concept_registers_,
         IndexList<runir::kr::dl::Register<runir::kr::dl::RoleTag>> role_registers_,
         IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> concept_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>> role_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::BooleanFeature>> boolean_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::NumericalFeature>> numerical_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>> query_features_,
         Index<runir::kr::ps::icp::MemoryState> entry_memory_state_,
         IndexList<runir::kr::ps::icp::MemoryState> memory_states_,
         IndexMatrix<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>> memory_transitions_,
         ::cista::offset::vector<runir::kr::ps::icp::ResetPair> reset_pairs_) :
        index(),
        symbol(std::move(symbol_)),
        concept_registers(std::move(concept_registers_)),
        role_registers(std::move(role_registers_)),
        concept_features(std::move(concept_features_)),
        role_features(std::move(role_features_)),
        boolean_features(std::move(boolean_features_)),
        numerical_features(std::move(numerical_features_)),
        query_features(std::move(query_features_)),
        entry_memory_state(std::move(entry_memory_state_)),
        memory_states(std::move(memory_states_)),
        memory_transitions(std::move(memory_transitions_)),
        reset_pairs(std::move(reset_pairs_))
    {
    }
    template<typename C, typename D>
    Data(::ygg::View<Index<runir::kr::ps::icp::ModuleSymbol>, C> symbol_,
         const std::vector<::ygg::View<Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>, D>>& concept_registers_,
         const std::vector<::ygg::View<Index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>, D>>& role_registers_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>, C>>& concept_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>>, C>>& role_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::BooleanFeature>>, C>>& boolean_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::NumericalFeature>>, C>>& numerical_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>>, C>>& query_features_,
         ::ygg::View<Index<runir::kr::ps::icp::MemoryState>, C> entry_memory_state_,
         const std::vector<::ygg::View<Index<runir::kr::ps::icp::MemoryState>, C>>& memory_states_,
         const std::vector<std::vector<::ygg::View<Index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>, C>>>& memory_transitions_,
         ::cista::offset::vector<runir::kr::ps::icp::ResetPair> reset_pairs_) :
        index(),
        symbol(),
        concept_registers(),
        role_registers(),
        concept_features(),
        role_features(),
        boolean_features(),
        numerical_features(),
        query_features(),
        entry_memory_state(),
        memory_states(),
        memory_transitions(),
        reset_pairs(std::move(reset_pairs_))
    {
        set(symbol_, symbol);
        set(concept_registers_, concept_registers);
        set(role_registers_, role_registers);
        set(concept_features_, concept_features);
        set(role_features_, role_features);
        set(boolean_features_, boolean_features);
        set(numerical_features_, numerical_features);
        set(query_features_, query_features);
        set(entry_memory_state_, entry_memory_state);
        set(memory_states_, memory_states);
        memory_transitions.reserve(memory_transitions_.size());
        for (const auto& row : memory_transitions_)
        {
            memory_transitions.emplace_back();
            set(row, memory_transitions.back());
        }
    }

    auto cista_members() noexcept
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
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
