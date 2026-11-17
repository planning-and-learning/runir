#ifndef PYRUNIR_KR_PS_BASE_DL_OBSERVATION_HPP_
#define PYRUNIR_KR_PS_BASE_DL_OBSERVATION_HPP_

#include "module.hpp"
#include "pyrunir/kr/ps/dl/binding_utils.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/dl/compatibility.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>

namespace runir::kr::ps::base::dl
{

/// The view of observation T (a condition or an effect, bound by bind_data first), its insertion and its compatibility
/// with ground and lifted transitions: is_compatible_with(observation, context).
template<typename T>
void bind_observation(nb::module_& m, RepositoryBinding& repository)
{
    using View = urd::index_view<T, Repository>;
    using GroundContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
    urd::python::bind_view<View>(m);
    ygg::bind_insert<T>(repository);
    m.def(
         "is_compatible_with",
         [](View observation, GroundContext& context) { return runir::kr::ps::is_compatible_with<tyr::GroundTag>(observation, context); },
         nb::arg("observation"),
         nb::arg("context"))
        .def(
            "is_compatible_with",
            [](View observation, LiftedContext& context) { return runir::kr::ps::is_compatible_with<tyr::LiftedTag>(observation, context); },
            nb::arg("observation"),
            nb::arg("context"));
}

}  // namespace runir::kr::ps::base::dl

#endif
