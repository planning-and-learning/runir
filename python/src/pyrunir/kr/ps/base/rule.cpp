#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/base/compatibility.hpp>
#include <runir/kr/ps/base/dl/evaluation_context.hpp>
#include <runir/kr/ps/base/formatter.hpp>
#include <runir/kr/ps/base/repository.hpp>
#include <runir/kr/ps/base/rule_data.hpp>
#include <runir/kr/ps/base/rule_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::base
{

using namespace nanobind::literals;

void bind_rule(nb::module_& m, RepositoryBinding& repository)
{
    using T = Rule;
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    using GroundContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;

    ygg::bind_index<ygg::Index<T>>(m, "RuleIndex");

    auto data = nb::class_<Data>(m, "RuleData")
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("symbol", &Data::symbol)
                    .def_rw("conditions", &Data::conditions)
                    .def_rw("effects", &Data::effects);
    ygg::add_comparison(data);

    auto view = nb::class_<View>(m, "Rule")
                    .def("get_index", &View::get_index)
                    .def("get_symbol", &View::get_symbol)
                    .def("get_conditions", &View::get_conditions)
                    .def("get_effects", &View::get_effects);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    view.def("is_compatible_with",
             [](View value, GroundContext& context) { return runir::kr::ps::base::is_compatible_with(value, context); },
             "context"_a)
        .def("is_compatible_with",
             [](View value, LiftedContext& context) { return runir::kr::ps::base::is_compatible_with(value, context); },
             "context"_a);

    repository.def("get_or_create", &runir::kr::python::get_or_create_data<T, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::base
