#ifndef RUNIR_KR_PS_EXT_MODULE_DATA_HPP_
#define RUNIR_KR_PS_EXT_MODULE_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace urd
{

template<>
struct data<runir::kr::ps::ext::Module>
{
    urd::key<runir::kr::ps::ext::Module> index;
    urd::index<runir::kr::ps::ModuleSymbol<runir::kr::ExtFamilyTag>> symbol;
    urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>> concept_arguments;
    urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::RoleTag>>> role_arguments;
    urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>> boolean_arguments;
    urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>> numerical_arguments;
    urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> concept_registers;
    urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>> role_registers;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>> concept_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>> role_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>> boolean_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>> numerical_features;
    urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>> query_features;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> entry_memory_state;
    urd::flat_vector<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>> memory_states;
    urd::flat_vector<urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>>> memory_transitions;
    /// The rules of its :datalog section as written (empty without one): printed back, so that the module parses again.
    urd::flat_string datalog;
    data() = default;

    // Arguments and registers live in the description-logic repository, hence the separate context parameter.
    data(urd::index<runir::kr::ps::ModuleSymbol<runir::kr::ExtFamilyTag>> symbol_,
         urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>> concept_arguments_,
         urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::RoleTag>>> role_arguments_,
         urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>> boolean_arguments_,
         urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>> numerical_arguments_,
         urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>> concept_registers_,
         urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>> role_registers_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>> concept_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>> role_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>> boolean_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>> numerical_features_,
         urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>> query_features_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> entry_memory_state_,
         urd::flat_vector<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>> memory_states_,
         urd::flat_vector<urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>>> memory_transitions_,
         urd::flat_string datalog_) :
        index(),
        symbol(symbol_),
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
        entry_memory_state(entry_memory_state_),
        memory_states(std::move(memory_states_)),
        memory_transitions(std::move(memory_transitions_)),
        datalog(std::move(datalog_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) noexcept = default;
    data& operator=(data&& other) noexcept = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"symbol", &data::symbol>,
                           urd::field<"concept_arguments", &data::concept_arguments>,
                           urd::field<"role_arguments", &data::role_arguments>,
                           urd::field<"boolean_arguments", &data::boolean_arguments>,
                           urd::field<"numerical_arguments", &data::numerical_arguments>,
                           urd::field<"concept_registers", &data::concept_registers>,
                           urd::field<"role_registers", &data::role_registers>,
                           urd::field<"concept_features", &data::concept_features>,
                           urd::field<"role_features", &data::role_features>,
                           urd::field<"boolean_features", &data::boolean_features>,
                           urd::field<"numerical_features", &data::numerical_features>,
                           urd::field<"query_features", &data::query_features>,
                           urd::field<"entry_memory_state", &data::entry_memory_state>,
                           urd::field<"memory_states", &data::memory_states>,
                           urd::field<"memory_transitions", &data::memory_transitions>,
                           urd::field<"datalog", &data::datalog>> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&data::symbol,
                            &data::concept_arguments,
                            &data::role_arguments,
                            &data::boolean_arguments,
                            &data::numerical_arguments,
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
                            &data::datalog> {};
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&data::index,
                            &data::symbol,
                            &data::concept_arguments,
                            &data::role_arguments,
                            &data::boolean_arguments,
                            &data::numerical_arguments,
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
                            &data::datalog> {};
    }

    [[nodiscard]] urd::key<runir::kr::ps::ext::Module> key() const noexcept { return urd::key<runir::kr::ps::ext::Module>(index); }
};

}  // namespace urd

#endif
