#include "bindings.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>
#include "pyrunir/kr/binding_utils.hpp"

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
#include <tyr/planning/state_repository.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::ps::base
{

void bind_rule(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    using T = runir::kr::ps::Rule<runir::kr::BaseFamilyTag>;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    using GroundContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>;
    using LiftedContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>;

    urd::python::bind_index<urd::index<T>>(m, "RuleIndex");

    using ConditionIndex = runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>;
    using EffectIndex = runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>;
    auto data = nb::class_<Data>(m, "RuleData")
                    .def(nb::init<>())
                    .def(nb::init<urd::flat_string,
                                  runir::kr::ps::ConditionList<runir::kr::BaseFamilyTag>,
                                  runir::kr::ps::EffectList<runir::kr::BaseFamilyTag>>(),
                         "symbol"_a,
                         "conditions"_a,
                         "effects"_a)
                    .def(urd::python::init_from_views<urd::flat_string,
                                  const urd::Store<urd::sequence<urd::resolved_t<ConditionIndex, Repository>>>&,
                                  const urd::Store<urd::sequence<urd::resolved_t<EffectIndex, Repository>>>&>(),
                         "symbol"_a,
                         "conditions"_a,
                         "effects"_a)
                    .def_rw("index", &Data::index)
                    .def_rw("symbol", &Data::symbol)
                    .def_rw("conditions", &Data::conditions)
                    .def_rw("effects", &Data::effects);
    urd::python::add_comparison(data);

    auto view = nb::class_<View>(m, "Rule")
                    .def("get_index", &View::index)
                    .def("get_symbol", &View::get_symbol)
                    .def("get_conditions", &View::get_conditions)
                    .def("get_effects", &View::get_effects);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    view.def(
            "is_compatible_with",
            [](View value, GroundContext& context) { return runir::kr::ps::base::is_compatible_with<tyr::GroundTag>(value, context); },
            "context"_a)
        .def(
            "is_compatible_with",
            [](View value, LiftedContext& context) { return runir::kr::ps::base::is_compatible_with<tyr::LiftedTag>(value, context); },
            "context"_a);

    ygg::bind_insert<T>(repository);
}

}  // namespace runir::kr::ps::base
