#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/compatibility.hpp>
#include <runir/kr/ps/base/dl/evaluation_context.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/sketch_data.hpp>
#include <runir/kr/ps/base/sketch_view.hpp>
#include <runir/kr/ps/base/syntactic_complexity.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::base
{

using namespace nanobind::literals;

void bind_sketch(nb::module_& m, RepositoryBinding& repository)
{
    using T = Sketch;
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    using GroundContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::planning::LiftedTag>;

    ygg::bind_index<ygg::Index<T>>(m, "SketchIndex");

    auto data = nb::class_<Data>(m, "SketchData")
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("boolean_features", &Data::boolean_features)
                    .def_rw("numerical_features", &Data::numerical_features)
                    .def_rw("rules", &Data::rules);
    ygg::add_comparison(data);

    auto view = nb::class_<View>(m, "Sketch")
                    .def("get_index", &View::get_index)
                    .def("get_boolean_features", &View::template get_features<runir::kr::ps::dl::BooleanFeature>)
                    .def("get_numerical_features", &View::template get_features<runir::kr::ps::dl::NumericalFeature>)
                    .def("get_rules", &View::get_rules)
                    .def("is_compatible_with",
                         [](View value, GroundContext& context) { return runir::kr::ps::base::is_compatible_with(value, context); },
                         "context"_a)
                    .def("is_compatible_with",
                         [](View value, LiftedContext& context) { return runir::kr::ps::base::is_compatible_with(value, context); },
                         "context"_a)
                    .def("syntactic_complexity", [](View value) { return runir::kr::ps::base::syntactic_complexity(value); });
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);

    repository.def("get_or_create", &runir::kr::python::get_or_create_data<T, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::base
