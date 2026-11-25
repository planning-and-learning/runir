#ifndef RUNIR_KR_PS_ICP_EXECUTION_REPOSITORY_TYPES_HPP_
#define RUNIR_KR_PS_ICP_EXECUTION_REPOSITORY_TYPES_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/icp/execution_builder.hpp"
#include "runir/kr/ps/icp/execution_data.hpp"

#include <boost/mp11.hpp>
#include <tyr/declarations.hpp>
#include <tyr/planning/declarations.hpp>
#include <yggdrasil/formalism/builder.hpp>

namespace runir::kr::ps::icp
{

/// The builders and scratch records of the control records, and the scratch records of the denotation repository's
/// records, which interning a control record's builder interns there (its registers, its arguments).
struct ControlBuilders :
    boost::mp11::mp_apply<ygg::formalism::Builders, boost::mp11::mp_append<ControlRepositoryTypes, runir::kr::dl::semantics::DenotationRecordTypes>>
{
};

using ControlBuilder = ygg::formalism::BasicBuilderStorage<ControlRepository, ControlBuilders>;

/// The builders and scratch records of the execution records, and the scratch records of the control and denotation
/// repositories' records and of the planning states, which interning an execution record's builder interns there (its
/// activation, its frames; its planning state and its storages, in the state repository).
template<tyr::TaskKind Kind>
struct ExecutionBuilders :
    boost::mp11::mp_apply<
        ygg::formalism::Builders,
        boost::mp11::mp_append<
            ExecutionRepositoryTypes<Kind>,
            ControlRepositoryTypes,
            runir::kr::dl::semantics::DenotationRecordTypes,
            boost::mp11::
                mp_list<tyr::planning::State<Kind>, tyr::planning::FactStorage<Kind>, tyr::planning::AtomStorage<Kind>, tyr::planning::NumericStorage<Kind>>>>
{
};

template<tyr::TaskKind Kind>
using ExecutionBuilder = ygg::formalism::BasicBuilderStorage<runir::kr::ps::ExecutionRepository<runir::kr::IcpFamilyTag, Kind>, ExecutionBuilders<Kind>>;

/// The execution repositories of the icp family, by task kind.
using GroundExecutionRepository = runir::kr::ps::ExecutionRepository<runir::kr::IcpFamilyTag, tyr::GroundTag>;
using LiftedExecutionRepository = runir::kr::ps::ExecutionRepository<runir::kr::IcpFamilyTag, tyr::LiftedTag>;
using GroundExecutionBuilder = ExecutionBuilder<tyr::GroundTag>;
using LiftedExecutionBuilder = ExecutionBuilder<tyr::LiftedTag>;

}

#endif
