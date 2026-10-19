#ifndef PYRUNIR_KR_DL_EVALUATION_BINDINGS_HPP_
#define PYRUNIR_KR_DL_EVALUATION_BINDINGS_HPP_

#include <nanobind/nanobind.h>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/ext/evaluation.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::python
{

template<runir::kr::FamilyTag Family, typename View>
void bind_evaluate(nanobind::class_<View>& cls)
{
    namespace nb = nanobind;
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<Family, tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<Family, tyr::LiftedTag>;
    cls.def(
           "evaluate",
           [](const View& view, GroundContext& context) { return runir::kr::dl::semantics::evaluate<tyr::GroundTag>(view, context); },
           nb::arg("context"),
           nb::keep_alive<0, 2>())
        .def(
            "evaluate",
            [](const View& view, LiftedContext& context) { return runir::kr::dl::semantics::evaluate<tyr::LiftedTag>(view, context); },
            nb::arg("context"),
            nb::keep_alive<0, 2>());
}

template<runir::kr::FamilyTag Family, tyr::TaskKind Kind>
void bind_state_evaluation_context(nanobind::module_& m, const char* name)
{
    namespace nb = nanobind;
    using Context = runir::kr::dl::semantics::StateEvaluationContext<Family, Kind>;
    using Storage = runir::kr::dl::semantics::EvaluationStorage<Family>;
    using Caches = runir::kr::dl::semantics::DenotationCaches<Family>;
    auto cls = nb::class_<Context>(m, name);
    if constexpr (std::same_as<Family, runir::kr::ExtFamilyTag>)
    {
        cls.def(nb::new_([](tyr::planning::StateView<Kind> state,
                            runir::kr::dl::semantics::Builder& builder,
                            Storage& storage,
                            runir::kr::dl::semantics::CallArgumentsView arguments,
                            runir::kr::dl::semantics::RegisterValuesView registers) { return Context(state, builder, storage, arguments, registers); }),
                nb::arg("state"),
                nb::arg("builder"),
                nb::arg("storage"),
                nb::arg("arguments"),
                nb::arg("registers"),
                nb::keep_alive<0, 2>(),
                nb::keep_alive<0, 3>(),
                nb::keep_alive<0, 4>(),
                nb::keep_alive<0, 5>(),
                nb::keep_alive<0, 6>())
            .def(nb::new_([](tyr::planning::StateView<Kind> state,
                             runir::kr::dl::semantics::Builder& builder,
                             Caches& caches,
                             runir::kr::dl::semantics::DenotationRepository& repository,
                             Storage& intermediates,
                             runir::kr::dl::semantics::CallArgumentsView arguments,
                             runir::kr::dl::semantics::RegisterValuesView registers)
                          { return Context(state, builder, caches, repository, intermediates, arguments, registers); }),
                 nb::arg("state"),
                 nb::arg("builder"),
                 nb::arg("caches"),
                 nb::arg("repository"),
                 nb::arg("intermediates"),
                 nb::arg("arguments"),
                 nb::arg("registers"),
                 nb::keep_alive<0, 2>(),
                 nb::keep_alive<0, 3>(),
                 nb::keep_alive<0, 4>(),
                 nb::keep_alive<0, 5>(),
                 nb::keep_alive<0, 6>(),
                 nb::keep_alive<0, 7>(),
                 nb::keep_alive<0, 8>());
    }
    else
    {
        cls.def(nb::new_([](tyr::planning::StateView<Kind> state, runir::kr::dl::semantics::Builder& builder, Storage& storage)
                         { return Context(state, builder, storage); }),
                nb::arg("state"),
                nb::arg("builder"),
                nb::arg("storage"),
                nb::keep_alive<0, 2>(),
                nb::keep_alive<0, 3>(),
                nb::keep_alive<0, 4>())
            .def(nb::new_([](tyr::planning::StateView<Kind> state,
                             runir::kr::dl::semantics::Builder& builder,
                             Caches& caches,
                             runir::kr::dl::semantics::DenotationRepository& repository,
                             Storage& intermediates) { return Context(state, builder, caches, repository, intermediates); }),
                 nb::arg("state"),
                 nb::arg("builder"),
                 nb::arg("caches"),
                 nb::arg("repository"),
                 nb::arg("intermediates"),
                 nb::keep_alive<0, 2>(),
                 nb::keep_alive<0, 3>(),
                 nb::keep_alive<0, 4>(),
                 nb::keep_alive<0, 5>(),
                 nb::keep_alive<0, 6>());
    }
    cls.def("get_state", &Context::get_state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
}

template<runir::kr::FamilyTag Family>
void bind_evaluation_storage(nanobind::module_& m)
{
    namespace nb = nanobind;
    using Caches = runir::kr::dl::semantics::DenotationCaches<Family>;
    using Storage = runir::kr::dl::semantics::EvaluationStorage<Family>;
    nb::class_<Caches>(m, "DenotationCaches", "Memoized interned views; does not own result storage.")
        .def(nb::init<>())
        .def("reset_dynamic", &Caches::reset_dynamic)
        .def("reset_all", &Caches::reset_all);
    nb::class_<Storage>(m, "EvaluationStorage", "Reusable result repositories and matching memoization.")
        .def(nb::init<const runir::kr::dl::semantics::DenotationRepository&>(), nb::arg("prototype"))
        .def(
            "get_caches",
            [](Storage& self) -> Caches& { return self.get_caches(); },
            nb::rv_policy::reference_internal)
        .def("reset_dynamic", &Storage::reset_dynamic)
        .def("reset_all", &Storage::reset_all);
}
}

#endif
