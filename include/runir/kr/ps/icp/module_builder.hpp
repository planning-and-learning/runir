#ifndef RUNIR_KR_PS_ICP_MODULE_BUILDER_HPP_
#define RUNIR_KR_PS_ICP_MODULE_BUILDER_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/icp/module_data.hpp"
#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/reset_pair_builder.hpp"

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
struct builder<runir::kr::ps::icp::Module, C>
{
    urd::key<runir::kr::ps::icp::Module> index;
    urd::built_t<urd::index<runir::kr::ps::ModuleSymbol<runir::kr::IcpFamilyTag>>, C> symbol;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>>, ::runir::kr::dl::ConstructorRepository> concept_registers;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>>, ::runir::kr::dl::ConstructorRepository> role_registers;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>>, C> concept_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>>>, C> role_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::BooleanTag>>>, C> boolean_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::NumericalTag>>>, C> numerical_features;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::ps::dl::QueryFeature>>>, C> query_features;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>, C> entry_memory_state;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::ps::MemoryState<runir::kr::IcpFamilyTag>>>, C> memory_states;
    urd::built_t<urd::flat_vector<urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::IcpFamilyTag>>>>, C> memory_transitions;
    urd::built_t<urd::flat_vector<urd::data<runir::kr::ps::icp::ResetPair>>, C> reset_pairs;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"symbol", &builder::symbol>,
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
                           urd::field<"reset_pairs", &builder::reset_pairs>> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&builder::symbol,
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
                            &builder::reset_pairs> {};
    }
};

}  // namespace urd

#endif
