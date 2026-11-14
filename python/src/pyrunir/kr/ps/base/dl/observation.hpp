#ifndef PYRUNIR_KR_PS_BASE_DL_OBSERVATION_HPP_
#define PYRUNIR_KR_PS_BASE_DL_OBSERVATION_HPP_

#include "module.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/compatibility.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>

namespace runir::kr::ps::base::dl
{

/// Binds observation T (a condition or an effect) with its compatibility with ground and lifted transitions.
template<typename T>
void bind_observation(nb::module_& m, RepositoryBinding& repository, const char* name)
{
    using View = urd::index_view<T, Repository>;
    using GroundContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
    runir::kr::python::bind_observation<T>(m, repository, name)
        .def(
            "is_compatible_with",
            [](View value, GroundContext& context) { return runir::kr::ps::is_compatible_with<tyr::GroundTag>(value, context); },
            nb::arg("context"))
        .def(
            "is_compatible_with",
            [](View value, LiftedContext& context) { return runir::kr::ps::is_compatible_with<tyr::LiftedTag>(value, context); },
            nb::arg("context"));
}

}  // namespace runir::kr::ps::base::dl

#endif
