#ifndef RUNIR_KR_PS_EXT_MODULE_DATA_HPP_
#define RUNIR_KR_PS_EXT_MODULE_DATA_HPP_

#include "runir/kr/dl/argument_index.hpp"
#include "runir/kr/dl/register_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/module_index.hpp"
#include "runir/kr/ps/ext/module_symbol_index.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"

#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace ygg
{

template<>
struct Data<runir::kr::ps::ext::Module>
{
    Index<runir::kr::ps::ext::Module> index;
    Index<runir::kr::ps::ext::ModuleSymbol> symbol;
    IndexList<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>> concept_arguments;
    IndexList<runir::kr::dl::Argument<runir::kr::dl::RoleTag>> role_arguments;
    IndexList<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>> boolean_arguments;
    IndexList<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>> numerical_arguments;
    IndexList<runir::kr::dl::Register<runir::kr::dl::ConceptTag>> concept_registers;
    IndexList<runir::kr::dl::Register<runir::kr::dl::RoleTag>> role_registers;
    IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>> concept_features;
    IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>> role_features;
    IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>> boolean_features;
    IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>> numerical_features;
    IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>> query_features;
    Index<runir::kr::ps::ext::MemoryState> entry_memory_state;
    IndexList<runir::kr::ps::ext::MemoryState> memory_states;
    IndexMatrix<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>> memory_transitions;
    Data() = default;
    // Arguments and registers live in the description-logic repository, hence the separate context parameter.
    Data(Index<runir::kr::ps::ext::ModuleSymbol> symbol_,
         IndexList<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>> concept_arguments_,
         IndexList<runir::kr::dl::Argument<runir::kr::dl::RoleTag>> role_arguments_,
         IndexList<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>> boolean_arguments_,
         IndexList<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>> numerical_arguments_,
         IndexList<runir::kr::dl::Register<runir::kr::dl::ConceptTag>> concept_registers_,
         IndexList<runir::kr::dl::Register<runir::kr::dl::RoleTag>> role_registers_,
         IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>> concept_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>> role_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>> boolean_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>> numerical_features_,
         IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>> query_features_,
         Index<runir::kr::ps::ext::MemoryState> entry_memory_state_,
         IndexList<runir::kr::ps::ext::MemoryState> memory_states_,
         IndexMatrix<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>> memory_transitions_) :
        index(),
        symbol(std::move(symbol_)),
        concept_arguments(std::move(concept_arguments_)),
        role_arguments(std::move(role_arguments_)),
        boolean_arguments(std::move(boolean_arguments_)),
        numerical_arguments(std::move(numerical_arguments_)),
        concept_registers(std::move(concept_registers_)),
        role_registers(std::move(role_registers_)),
        concept_features(std::move(concept_features_)),
        role_features(std::move(role_features_)),
        boolean_features(std::move(boolean_features_)),
        numerical_features(std::move(numerical_features_)),
        query_features(std::move(query_features_)),
        entry_memory_state(std::move(entry_memory_state_)),
        memory_states(std::move(memory_states_)),
        memory_transitions(std::move(memory_transitions_))
    {
    }
    template<typename C, typename D>
    Data(::ygg::View<Index<runir::kr::ps::ext::ModuleSymbol>, C> symbol_,
         const std::vector<::ygg::View<Index<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>, D>>& concept_arguments_,
         const std::vector<::ygg::View<Index<runir::kr::dl::Argument<runir::kr::dl::RoleTag>>, D>>& role_arguments_,
         const std::vector<::ygg::View<Index<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>, D>>& boolean_arguments_,
         const std::vector<::ygg::View<Index<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>, D>>& numerical_arguments_,
         const std::vector<::ygg::View<Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>, D>>& concept_registers_,
         const std::vector<::ygg::View<Index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>, D>>& role_registers_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>, C>>& concept_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>, C>>& role_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>, C>>& boolean_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>, C>>& numerical_features_,
         const std::vector<::ygg::View<Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>, C>>& query_features_,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C> entry_memory_state_,
         const std::vector<::ygg::View<Index<runir::kr::ps::ext::MemoryState>, C>>& memory_states_,
         const std::vector<std::vector<::ygg::View<Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>, C>>>& memory_transitions_) :
        index(),
        symbol(),
        concept_arguments(),
        role_arguments(),
        boolean_arguments(),
        numerical_arguments(),
        concept_registers(),
        role_registers(),
        concept_features(),
        role_features(),
        boolean_features(),
        numerical_features(),
        query_features(),
        entry_memory_state(),
        memory_states(),
        memory_transitions()
    {
        set(symbol_, symbol);
        set(concept_arguments_, concept_arguments);
        set(role_arguments_, role_arguments);
        set(boolean_arguments_, boolean_arguments);
        set(numerical_arguments_, numerical_arguments);
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
                        concept_arguments,
                        role_arguments,
                        boolean_arguments,
                        numerical_arguments,
                        concept_registers,
                        role_registers,
                        concept_features,
                        role_features,
                        boolean_features,
                        numerical_features,
                        query_features,
                        entry_memory_state,
                        memory_states,
                        memory_transitions);
    }
    auto cista_members() const noexcept
    {
        return std::tie(index,
                        symbol,
                        concept_arguments,
                        role_arguments,
                        boolean_arguments,
                        numerical_arguments,
                        concept_registers,
                        role_registers,
                        concept_features,
                        role_features,
                        boolean_features,
                        numerical_features,
                        query_features,
                        entry_memory_state,
                        memory_states,
                        memory_transitions);
    }
    auto identifying_members() const noexcept
    {
        return std::tie(symbol,
                        concept_arguments,
                        role_arguments,
                        boolean_arguments,
                        numerical_arguments,
                        concept_registers,
                        role_registers,
                        concept_features,
                        role_features,
                        boolean_features,
                        numerical_features,
                        query_features,
                        entry_memory_state,
                        memory_states,
                        memory_transitions);
    }
    void clear() noexcept
    {
        std::apply([](auto&... member) { (ygg::clear(member), ...); }, cista_members());
    }
};

}  // namespace ygg

#endif
