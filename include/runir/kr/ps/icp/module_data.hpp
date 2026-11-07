#ifndef RUNIR_KR_PS_ICP_MODULE_DATA_HPP_
#define RUNIR_KR_PS_ICP_MODULE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/icp/declarations.hpp"
#include "runir/kr/ps/icp/object_reference.hpp"
#include "runir/kr/ps/icp/rule_data.hpp"

#include <cista/containers/vector.h>
#include <tuple>
#include <utility>
#include <vector>
#include <urd/elements/flat.hpp>

namespace urd
{

template<>
struct data<runir::kr::ps::icp::Module>
{
    urd::index<runir::kr::ps::icp::Module> index;
    urd::index<runir::kr::ps::ModuleSymbol<runir::kr::IcpFamilyTag>> symbol;
    urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> concept_registers;
    urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>> role_registers;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>> concept_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>>> role_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::BooleanTag>>> boolean_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::NumericalTag>>> numerical_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>>> query_features;
    urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> entry_memory_state;
    urd::flat_vector<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>> memory_states;
    urd::flat_vector<urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>>> memory_transitions;
    urd::flat_vector<runir::kr::ps::icp::ResetPair> reset_pairs;

    data() = default;
    // Registers live in the description-logic repository, hence the separate context parameter.
    data(urd::index<runir::kr::ps::ModuleSymbol<runir::kr::IcpFamilyTag>> symbol_,
         urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> concept_registers_,
         urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>> role_registers_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>> concept_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>>> role_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::BooleanTag>>> boolean_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::NumericalTag>>> numerical_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>>> query_features_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>> entry_memory_state_,
         urd::flat_vector<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>> memory_states_,
         urd::flat_vector<urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>>> memory_transitions_,
         urd::flat_vector<runir::kr::ps::icp::ResetPair> reset_pairs_) :
        index(),
        symbol(symbol_),
        concept_registers(std::move(concept_registers_)),
        role_registers(std::move(role_registers_)),
        concept_features(std::move(concept_features_)),
        role_features(std::move(role_features_)),
        boolean_features(std::move(boolean_features_)),
        numerical_features(std::move(numerical_features_)),
        query_features(std::move(query_features_)),
        entry_memory_state(entry_memory_state_),
        memory_states(std::move(memory_states_)),
        memory_transitions(std::move(memory_transitions_)),
        reset_pairs(std::move(reset_pairs_))
    {
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&data::index,
                            &data::symbol,
                            &data::concept_registers,
                            &data::role_registers,
                            &data::concept_features,
                            &data::role_features,
                            &data::boolean_features,
                            &data::numerical_features,
                            &data::query_features,
                            &data::entry_memory_state,
                            &data::memory_states,
                            &data::memory_transitions,
                            &data::reset_pairs> {};
    }
    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&data::symbol,
                            &data::concept_registers,
                            &data::role_registers,
                            &data::concept_features,
                            &data::role_features,
                            &data::boolean_features,
                            &data::numerical_features,
                            &data::query_features,
                            &data::entry_memory_state,
                            &data::memory_states,
                            &data::memory_transitions,
                            &data::reset_pairs> {};
    }
};

}  // namespace urd

#endif
