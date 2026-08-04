#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/uns/evaluation_context.hpp>
#include <runir/kr/uns/dl/evaluation.hpp>
#include <runir/kr/uns/dl/feature_data.hpp>
#include <runir/kr/uns/dl/feature_index.hpp>
#include <runir/kr/uns/dl/feature_view.hpp>
#include <runir/kr/uns/dl/formatter.hpp>
#include <runir/kr/uns/dl/syntactic_complexity.hpp>
#include <runir/kr/uns/repository.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::uns::dl
{

using namespace nanobind::literals;

void bind_feature(nb::module_& m)
{
    using Type = runir::kr::uns::dl::Feature;
    using Data = ygg::Data<Type>;
    using View = ygg::View<ygg::Index<Type>, runir::kr::uns::Repository>;
    using GroundContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>;

    ygg::bind_index<ygg::Index<Type>>(m, "FeatureIndex");

    auto data = nb::class_<Data>(m, "FeatureData")
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("feature", &Data::feature)
                    .def_rw("symbol", &Data::symbol);
    ygg::add_comparison(data);

    auto view = nb::class_<View>(m, "Feature")
                    .def("get_index", &View::get_index)
                    .def("get_expression", &View::get_expression, nb::keep_alive<0, 1>())
                    .def("get_feature", &View::get_feature, nb::keep_alive<0, 1>())
                    .def("get_symbol", &View::get_symbol)
                    .def("syntactic_complexity", [](View value) { return syntactic_complexity(value); })
                    .def("evaluate", [](View value, GroundContext& context) { return evaluate(value, context); }, "context"_a)
                    .def("evaluate", [](View value, LiftedContext& context) { return evaluate(value, context); }, "context"_a);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
}

}  // namespace runir::kr::uns::dl
