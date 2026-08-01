#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/base/evaluation_context.hpp>
#include <runir/kr/ps/base/dl/evaluation.hpp>
#include <runir/kr/ps/base/dl/feature_data.hpp>
#include <runir/kr/ps/base/dl/feature_view.hpp>
#include <runir/kr/ps/base/dl/syntactic_complexity.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <runir/kr/ps/feature_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::base::dl
{

using namespace nanobind::literals;

void bind_numerical_feature(nb::module_& m, RepositoryBinding& repository)
{
    using Feature = runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>;
    using FeatureData = ygg::Data<Feature>;
    using FeatureView = ygg::View<ygg::Index<Feature>, Repository>;
    using ConcreteFeature = runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>;
    using ConcreteFeatureData = ygg::Data<ConcreteFeature>;
    using ConcreteFeatureView = ygg::View<ygg::Index<ConcreteFeature>, Repository>;
    using GroundContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>;

    ygg::bind_index<ygg::Index<Feature>>(m, "NumericalFeatureIndex");
    ygg::bind_index<ygg::Index<ConcreteFeature>>(m, "ConcreteNumericalFeatureIndex");

    auto feature_data =
        nb::class_<FeatureData>(m, "NumericalFeatureData").def(nb::init<>()).def_rw("index", &FeatureData::index).def_rw("value", &FeatureData::value);
    ygg::add_comparison(feature_data);

    auto concrete_feature_data = nb::class_<ConcreteFeatureData>(m, "ConcreteNumericalFeatureData")
                                     .def(nb::init<>())
                                     .def_rw("index", &ConcreteFeatureData::index)
                                     .def_rw("feature", &ConcreteFeatureData::feature)
                                     .def_rw("symbol", &ConcreteFeatureData::symbol);
    ygg::add_comparison(concrete_feature_data);

    auto feature = nb::class_<FeatureView>(m, "NumericalFeature")
                       .def("get_index", &FeatureView::get_index)
                       .def("get_variant", &FeatureView::get_variant)
                       .def("get_feature", &FeatureView::get_feature, nb::keep_alive<0, 1>())
                       .def("get_expression", &FeatureView::get_expression, nb::keep_alive<0, 1>())
                       .def("get_symbol", &FeatureView::get_symbol)
                       .def(
                           "evaluate",
                           [](FeatureView value, GroundContext& context) { return runir::kr::ps::evaluate(value, context); },
                           "context"_a)
                       .def(
                           "evaluate",
                           [](FeatureView value, LiftedContext& context) { return runir::kr::ps::evaluate(value, context); },
                           "context"_a)
                       .def("syntactic_complexity", [](FeatureView value) { return runir::kr::ps::base::syntactic_complexity(value); });
    ygg::add_print(feature);
    ygg::add_comparison(feature);
    ygg::add_hash(feature);

    auto concrete_feature = nb::class_<ConcreteFeatureView>(m, "ConcreteNumericalFeature")
                                .def("get_index", &ConcreteFeatureView::get_index)
                                .def("get_feature", &ConcreteFeatureView::get_feature, nb::keep_alive<0, 1>())
                                .def("get_expression", &ConcreteFeatureView::get_expression, nb::keep_alive<0, 1>())
                                .def("get_symbol", &ConcreteFeatureView::get_symbol)
                                .def(
                                    "evaluate",
                                    [](ConcreteFeatureView value, GroundContext& context) { return runir::kr::ps::evaluate(value, context); },
                                    "context"_a)
                                .def(
                                    "evaluate",
                                    [](ConcreteFeatureView value, LiftedContext& context) { return runir::kr::ps::evaluate(value, context); },
                                    "context"_a)
                                .def("syntactic_complexity", [](ConcreteFeatureView value) { return runir::kr::ps::base::dl::syntactic_complexity(value); });
    ygg::add_print(concrete_feature);
    ygg::add_comparison(concrete_feature);
    ygg::add_hash(concrete_feature);

    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Feature, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<ConcreteFeature, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::base::dl
