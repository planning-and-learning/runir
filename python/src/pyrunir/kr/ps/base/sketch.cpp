#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <nanobind/stl/vector.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/base/compatibility.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_data.hpp>
#include <runir/kr/ps/base/sketch_view.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <runir/kr/ps/dl/transition_evaluation_context.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::base
{

void bind_sketch(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    using T = Sketch;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    using GroundContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;

    urd::python::bind_index<urd::index<T>>(m, "SketchIndex");

    using BooleanFeatureIndex = urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>;
    using NumericalFeatureIndex = urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>;
    using RuleIndex = urd::index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>;
    auto data =
        nb::class_<Data>(m, "SketchData")
            .def(nb::init<>())
            .def(nb::init<urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>>>,
                          urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>>>,
                          urd::flat_vector<urd::index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>>>(),
                 "boolean_features"_a,
                 "numerical_features"_a,
                 "rules"_a)
            .def(urd::python::init_from_views<
                     const urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::BooleanTag>, Repository>>>&,
                     const urd::Store<
                         urd::sequence<urd::index_view<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::dl::NumericalTag>, Repository>>>&,
                     const urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>, Repository>>>&>(),
                 "boolean_features"_a,
                 "numerical_features"_a,
                 "rules"_a)
            .def_rw("index", &Data::index)
            .def_rw("boolean_features", &Data::boolean_features)
            .def_rw("numerical_features", &Data::numerical_features)
            .def_rw("rules", &Data::rules);
    urd::python::add_comparison(data);

    auto view = nb::class_<View>(m, "Sketch")
                    .def("index", &View::index)
                    .def("get_boolean_features", &View::template get_features<runir::kr::dl::BooleanTag>)
                    .def("get_numerical_features", &View::template get_features<runir::kr::dl::NumericalTag>)
                    .def("get_rules", &View::get_rules)
                    .def(
                        "is_compatible_with",
                        [](View value, GroundContext& context) { return runir::kr::ps::base::is_compatible_with<tyr::GroundTag>(value, context); },
                        "context"_a)
                    .def(
                        "is_compatible_with",
                        [](View value, LiftedContext& context) { return runir::kr::ps::base::is_compatible_with<tyr::LiftedTag>(value, context); },
                        "context"_a)
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::base::syntactic_complexity(value); });
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    runir::kr::python::bind_ps_optimize<runir::kr::BaseFamilyTag, Repository>(view);

    ygg::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::base
