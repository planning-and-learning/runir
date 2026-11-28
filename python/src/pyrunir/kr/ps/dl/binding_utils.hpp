#ifndef PYRUNIR_KR_PS_DL_BINDING_UTILS_HPP_
#define PYRUNIR_KR_PS_DL_BINDING_UTILS_HPP_

#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/kr/ps/base/conversions.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"
#include "pyrunir/kr/ps/ext/conversions.hpp"
#include "pyrunir/kr/ps/icp/conversions.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/ps/dl/declarations.hpp>
#include <runir/kr/ps/dl/effect_data.hpp>
#include <runir/kr/ps/dl/evaluation.hpp>
#include <runir/kr/ps/evaluation.hpp>
#include <runir/kr/ps/family_traits.hpp>
#include <runir/kr/ps/feature_data.hpp>
#include <runir/kr/ps/formatter.hpp>
#include <runir/kr/ps/syntactic_complexity.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <urd/python/bindings.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::python
{

namespace nb = nanobind;

/// evaluate(feature, context) of a feature view (a feature or a concrete feature), in a ground or a lifted state.
template<typename View>
void bind_feature_evaluate(nb::module_& m)
{
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>;
    m.def(
         "evaluate",
         [](View feature, GroundContext& context) { return runir::kr::ps::evaluate<tyr::GroundTag>(feature, context); },
         nb::arg("feature"),
         nb::arg("context"),
         nb::keep_alive<0, 2>())
        .def(
            "evaluate",
            [](View feature, LiftedContext& context) { return runir::kr::ps::evaluate<tyr::LiftedTag>(feature, context); },
            nb::arg("feature"),
            nb::arg("context"),
            nb::keep_alive<0, 2>());
}

/// The views of the feature of Family and FeatureTag and of its concrete (DL) feature, whose records are bound first
/// (bind_data), their insertion, the feature's expression, and optimize and the query
/// statistics of a feature.
template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename Repository>
void bind_feature_views(nb::module_& m, nb::class_<Repository>& repository)
{
    using FeatureView = urd::index_view<runir::kr::ps::Feature<Family, FeatureTag>, Repository>;
    using ConcreteView = urd::index_view<runir::kr::ps::ConcreteFeature<Family, FeatureTag>, Repository>;

    urd::python::bind_view<FeatureView>(m);
    urd::python::bind_builder<runir::kr::ps::Feature<Family, FeatureTag>, Repository>(m);
    urd::python::bind_view<ConcreteView>(m);
    urd::python::bind_builder<runir::kr::ps::ConcreteFeature<Family, FeatureTag>, Repository>(m);
    bind_ps_optimize<Family, Repository, FeatureView>(m);
    ygg::bind_insert<runir::kr::ps::Feature<Family, FeatureTag>>(repository);
    ygg::bind_insert<runir::kr::ps::ConcreteFeature<Family, FeatureTag>>(repository);
}

}  // namespace runir::kr::python

#endif
