#include "bindings.hpp"
#include "pyrunir/conversions.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/base/compatibility.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_data.hpp>
#include <runir/kr/ps/base/sketch_view.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::base
{

void bind_sketch(nb::module_& m, RepositoryBinding& repository)
{
    using View = SketchView;
    using GroundContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
    urd::python::bind_index<Sketch>(m);
    urd::python::bind_data<Sketch>(m);
    urd::python::bind_view<View>(m);
    urd::python::bind_builder<runir::kr::ps::base::Sketch, Repository>(m);
    m.def(
         "is_compatible_with",
         [](View sketch, GroundContext& context) { return runir::kr::ps::base::is_compatible_with<tyr::GroundTag>(sketch, context); },
         nb::arg("sketch"),
         nb::arg("context"))
        .def(
            "is_compatible_with",
            [](View sketch, LiftedContext& context) { return runir::kr::ps::base::is_compatible_with<tyr::LiftedTag>(sketch, context); },
            nb::arg("sketch"),
            nb::arg("context"));
    runir::kr::python::bind_ps_optimize<runir::kr::BaseFamilyTag, Repository, View>(m);
    ygg::bind_insert<Sketch>(repository);
}

}  // namespace runir::kr::ps::base
