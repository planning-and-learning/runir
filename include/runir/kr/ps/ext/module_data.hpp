#ifndef RUNIR_KR_PS_EXT_MODULE_DATA_HPP_
#define RUNIR_KR_PS_EXT_MODULE_DATA_HPP_

#include "runir/kr/dl/argument_index.hpp"
#include "runir/kr/dl/register_index.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/module_index.hpp"
#include "runir/kr/ps/ext/module_symbol_index.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_index.hpp"
#include "runir/kr/ps/ext/rule_variant_index.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <tuple>
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
    Index<runir::kr::ps::ext::MemoryState> entry_memory_state;
    IndexList<runir::kr::ps::ext::MemoryState> memory_states;
    IndexMatrix<runir::kr::ps::ext::RuleVariant> memory_transitions;
    Data() = default;
    Data(Index<runir::kr::ps::ext::ModuleSymbol> symbol_) : index(), symbol(symbol_) {}

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(symbol);
        ygg::clear(concept_arguments);
        ygg::clear(role_arguments);
        ygg::clear(boolean_arguments);
        ygg::clear(numerical_arguments);
        ygg::clear(concept_registers);
        ygg::clear(role_registers);
        ygg::clear(concept_features);
        ygg::clear(role_features);
        ygg::clear(boolean_features);
        ygg::clear(numerical_features);
        ygg::clear(entry_memory_state);
        ygg::clear(memory_states);
        ygg::clear(memory_transitions);
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
                        entry_memory_state,
                        memory_states,
                        memory_transitions);
    }
};

}  // namespace ygg

#endif
