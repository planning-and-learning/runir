#include "module.hpp"

#include "pyrunir/kr/ps/base/module.hpp"
#include "pyrunir/kr/ps/ext/module.hpp"
#include "pyrunir/kr/ps/icp/module.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/ps/dl/incomplete_structural_termination_data.hpp>
#include <runir/kr/ps/proof_status.hpp>

namespace runir::kr::ps
{

void bind_module_definitions(nb::module_& m)
{
    nb::enum_<ProofStatus>(m, "ProofStatus")
        .value("SUCCESS", ProofStatus::SUCCESS)
        .value("FAILURE", ProofStatus::FAILURE)
        .value("OUT_OF_TIME", ProofStatus::OUT_OF_TIME)
        .value("OUT_OF_STATES", ProofStatus::OUT_OF_STATES);

    nb::enum_<dl::IncompleteStructuralTerminationStatus>(m, "IncompleteStructuralTerminationStatus")
        .value("TERMINATING", dl::IncompleteStructuralTerminationStatus::TERMINATING)
        .value("UNKNOWN", dl::IncompleteStructuralTerminationStatus::UNKNOWN);

    auto base = m.def_submodule("base");
    runir::kr::ps::base::bind_module_definitions(base);

    auto ext = m.def_submodule("ext");
    runir::kr::ps::ext::bind_module_definitions(ext);

    auto icp = m.def_submodule("icp");
    runir::kr::ps::icp::bind_module_definitions(icp);
}

}  // namespace runir::kr::ps
