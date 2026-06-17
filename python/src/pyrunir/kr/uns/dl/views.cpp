#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/semantics/uns/evaluation_context.hpp>
#include <runir/kr/uns/dl/evaluation.hpp>
#include <runir/kr/uns/dl/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <string>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/ground_task/state_repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_repository.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::uns::dl
{

using namespace nanobind::literals;

namespace
{

using Family = runir::kr::UnsFamilyTag;

template<typename T>
void bind_view(nb::module_& m, const std::string& name)
{
    using View = ygg::View<ygg::Index<T>, runir::kr::uns::Repository>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_hash(cls);

    if constexpr (requires(const View& view) { view.get_expression(); })
        cls.def("get_expression", &View::get_expression);
    if constexpr (requires(const View& view) { view.get_feature(); })
        cls.def("get_feature", &View::get_feature);
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", [](const View& view) { return std::string(view.get_symbol()); });
    if constexpr (requires(const View& view) { view.get_description(); })
        cls.def("get_description", [](const View& view) { return std::string(view.get_description()); });
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<Family, tyr::planning::GroundTag>& context) { evaluate(view, context); })
        cls.def(
            "evaluate",
            [](const View& view, runir::kr::dl::semantics::EvaluationContext<Family, tyr::planning::GroundTag>& context) { return evaluate(view, context); },
            "context"_a);
    if constexpr (requires(const View& view, runir::kr::dl::semantics::EvaluationContext<Family, tyr::planning::LiftedTag>& context) { evaluate(view, context); })
        cls.def(
            "evaluate",
            [](const View& view, runir::kr::dl::semantics::EvaluationContext<Family, tyr::planning::LiftedTag>& context) { return evaluate(view, context); },
            "context"_a);
}

}  // namespace

void bind_views(nb::module_& m) { bind_view<runir::kr::uns::dl::Feature>(m, "Feature"); }

}  // namespace runir::kr::uns::dl
