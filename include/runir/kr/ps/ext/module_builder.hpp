#ifndef RUNIR_KR_PS_EXT_MODULE_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_MODULE_BUILDER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/module_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Module: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::Module, C>
{
    urd::key<runir::kr::ps::ext::Module> index;
    urd::built_t<urd::index<runir::kr::ps::ModuleSymbol<runir::kr::ExtFamilyTag>>, C> symbol;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>>>, ::runir::kr::dl::ConstructorRepository> concept_arguments;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::RoleTag>>>, ::runir::kr::dl::ConstructorRepository> role_arguments;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>>>, ::runir::kr::dl::ConstructorRepository> boolean_arguments;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Argument<runir::kr::dl::NumericalTag>>>, ::runir::kr::dl::ConstructorRepository>
        numerical_arguments;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>>, ::runir::kr::dl::ConstructorRepository> concept_registers;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>>, ::runir::kr::dl::ConstructorRepository> role_registers;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>, C> concept_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>>, C> role_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::BooleanTag>>>, C> boolean_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::NumericalTag>>>, C> numerical_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>>, C> query_features;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, C> entry_memory_state;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>>, C> memory_states;
    urd::built_t<urd::flat_vector<urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>>>, C> memory_transitions;
    urd::built_t<urd::flat_string, C> datalog;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"symbol", &builder::symbol>,
                           urd::field<"concept_arguments", &builder::concept_arguments>,
                           urd::field<"role_arguments", &builder::role_arguments>,
                           urd::field<"boolean_arguments", &builder::boolean_arguments>,
                           urd::field<"numerical_arguments", &builder::numerical_arguments>,
                           urd::field<"concept_registers", &builder::concept_registers>,
                           urd::field<"role_registers", &builder::role_registers>,
                           urd::field<"concept_features", &builder::concept_features>,
                           urd::field<"role_features", &builder::role_features>,
                           urd::field<"boolean_features", &builder::boolean_features>,
                           urd::field<"numerical_features", &builder::numerical_features>,
                           urd::field<"query_features", &builder::query_features>,
                           urd::field<"entry_memory_state", &builder::entry_memory_state>,
                           urd::field<"memory_states", &builder::memory_states>,
                           urd::field<"memory_transitions", &builder::memory_transitions>,
                           urd::field<"datalog", &builder::datalog>> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&builder::symbol,
                            &builder::concept_arguments,
                            &builder::role_arguments,
                            &builder::boolean_arguments,
                            &builder::numerical_arguments,
                            &builder::concept_registers,
                            &builder::role_registers,
                            &builder::concept_features,
                            &builder::role_features,
                            &builder::boolean_features,
                            &builder::numerical_features,
                            &builder::query_features,
                            &builder::entry_memory_state,
                            &builder::memory_states,
                            &builder::memory_transitions,
                            &builder::datalog> {};
    }
};

}  // namespace urd

#endif
