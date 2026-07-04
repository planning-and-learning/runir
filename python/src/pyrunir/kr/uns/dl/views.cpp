#include "module.hpp"

#include <nanobind/stl/string.h>
#include <runir/kr/dl/semantics/uns/evaluation_context.hpp>
#include <runir/kr/uns/dl/evaluation.hpp>
#include <runir/kr/uns/dl/formatter.hpp>
#include <runir/kr/uns/repository.hpp>
#include <string>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <tyr/planning/lifted/state_view.hpp>
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
    using GroundContext = runir::kr::dl::semantics::EvaluationContext<Family, tyr::planning::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::EvaluationContext<Family, tyr::planning::LiftedTag>;

    auto cls = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_hash(cls);

    if constexpr (requires(const View& view) { view.get_expression(); })
        cls.def("get_expression", &View::get_expression, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_feature(); })
        cls.def("get_feature", &View::get_feature, nb::keep_alive<0, 1>());
    if constexpr (requires(const View& view) { view.get_symbol(); })
        cls.def("get_symbol", &View::get_symbol);
    if constexpr (requires(const View& view, GroundContext& context) { evaluate(view, context); })
        cls.def("evaluate", nb::overload_cast<View, GroundContext&>(&evaluate), "context"_a);
    if constexpr (requires(const View& view, LiftedContext& context) { evaluate(view, context); })
        cls.def("evaluate", nb::overload_cast<View, LiftedContext&>(&evaluate), "context"_a);
}

}  // namespace

void bind_views(nb::module_& m) { bind_view<runir::kr::uns::dl::Feature>(m, "Feature"); }

}  // namespace runir::kr::uns::dl
