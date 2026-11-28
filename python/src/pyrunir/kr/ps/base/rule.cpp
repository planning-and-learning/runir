#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/ps/base/compatibility.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/rule_data.hpp>
#include <runir/kr/ps/base/rule_view.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::base
{

void bind_rule(nb::module_& m, RepositoryBinding& repository)
{
    using View = urd::index_view<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>, Repository>;
    using GroundContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
    urd::python::bind_index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>(m);
    urd::python::bind_data<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>(m);
    urd::python::bind_view<View>(m);
    m.def(
         "is_compatible_with",
         [](View rule, GroundContext& context) { return runir::kr::ps::base::is_compatible_with<tyr::GroundTag>(rule, context); },
         nb::arg("rule"),
         nb::arg("context"))
        .def(
            "is_compatible_with",
            [](View rule, LiftedContext& context) { return runir::kr::ps::base::is_compatible_with<tyr::LiftedTag>(rule, context); },
            nb::arg("rule"),
            nb::arg("context"));
    ygg::bind_insert<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>(repository);
}

}  // namespace runir::kr::ps::base
