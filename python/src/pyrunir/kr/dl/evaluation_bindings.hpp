#ifndef PYRUNIR_KR_DL_EVALUATION_BINDINGS_HPP_
#define PYRUNIR_KR_DL_EVALUATION_BINDINGS_HPP_

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <cstddef>
#include <nanobind/nanobind.h>
#include <runir/datasets/successors.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <tyr/planning/states/state_repository.hpp>

namespace runir::kr::python
{

/// evaluate(expression, context) of a DL expression view (a constructor or a query), in a ground or a lifted state.
template<typename View>
void bind_evaluate(nanobind::module_& m)
{
    namespace nb = nanobind;
    using GroundContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::GroundTag>;
    using LiftedContext = runir::kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>;
    m.def(
         "evaluate",
         [](const View& expression, GroundContext& context) { return runir::kr::dl::semantics::evaluate<tyr::GroundTag>(expression, context); },
         nb::arg("expression"),
         nb::arg("context"),
         nb::keep_alive<0, 2>())
        .def(
            "evaluate",
            [](const View& expression, LiftedContext& context) { return runir::kr::dl::semantics::evaluate<tyr::LiftedTag>(expression, context); },
            nb::arg("expression"),
            nb::arg("context"),
            nb::keep_alive<0, 2>());
}

template<tyr::TaskKind Kind>
void bind_state_evaluation_context(nanobind::module_& m, const char* name)
{
    namespace nb = nanobind;
    using Context = runir::kr::dl::semantics::StateEvaluationContext<Kind>;
    using Storage = runir::kr::dl::semantics::EvaluationStorage;
    using Caches = runir::kr::dl::semantics::DenotationCaches;
    auto cls = nb::class_<Context>(m, name);
    // Programs bind arguments and registers; other languages evaluate without them.
    // keep_alive<0, i>: the result (0) keeps constructor argument i alive; self is 1, the arguments start at 2.
    constexpr std::size_t first_argument = 2;
    constexpr std::size_t second_argument = 3;
    constexpr std::size_t third_argument = 4;
    constexpr std::size_t fourth_argument = 5;
    constexpr std::size_t fifth_argument = 6;
    constexpr std::size_t sixth_argument = 7;
    constexpr std::size_t seventh_argument = 8;
    cls.def(nb::new_([](const tyr::planning::StateBuilderView<Kind>& state, runir::kr::dl::semantics::Builder& builder, Storage& storage)
                     { return Context(runir::datasets::shared_state<Kind>(state), builder, storage); }),
            nb::arg("state"),
            nb::arg("builder"),
            nb::arg("storage"),
            nb::keep_alive<0, first_argument>(),
            nb::keep_alive<0, second_argument>(),
            nb::keep_alive<0, third_argument>());
    cls.def(nb::new_([](const tyr::planning::StateBuilderView<Kind>& state,
                        runir::kr::dl::semantics::Builder& builder,
                        Caches& caches,
                        runir::kr::dl::semantics::DenotationRepository& repository,
                        Storage& intermediates) { return Context(runir::datasets::shared_state<Kind>(state), builder, caches, repository, intermediates); }),
            nb::arg("state"),
            nb::arg("builder"),
            nb::arg("caches"),
            nb::arg("repository"),
            nb::arg("intermediates"),
            nb::keep_alive<0, first_argument>(),
            nb::keep_alive<0, second_argument>(),
            nb::keep_alive<0, third_argument>(),
            nb::keep_alive<0, fourth_argument>(),
            nb::keep_alive<0, fifth_argument>());
    cls.def(nb::new_([](const tyr::planning::StateBuilderView<Kind>& state,
                        runir::kr::dl::semantics::Builder& builder,
                        Storage& storage,
                        runir::kr::dl::semantics::CallArgumentsView arguments,
                        runir::kr::dl::semantics::RegisterValuesView registers)
                     { return Context(runir::datasets::shared_state<Kind>(state), builder, storage, arguments, registers); }),
            nb::arg("state"),
            nb::arg("builder"),
            nb::arg("storage"),
            nb::arg("arguments"),
            nb::arg("registers"),
            nb::keep_alive<0, first_argument>(),
            nb::keep_alive<0, second_argument>(),
            nb::keep_alive<0, third_argument>(),
            nb::keep_alive<0, fourth_argument>(),
            nb::keep_alive<0, fifth_argument>());
    cls.def(nb::new_([](const tyr::planning::StateBuilderView<Kind>& state,
                        runir::kr::dl::semantics::Builder& builder,
                        Caches& caches,
                        runir::kr::dl::semantics::DenotationRepository& repository,
                        Storage& intermediates,
                        runir::kr::dl::semantics::CallArgumentsView arguments,
                        runir::kr::dl::semantics::RegisterValuesView registers)
                     { return Context(runir::datasets::shared_state<Kind>(state), builder, caches, repository, intermediates, arguments, registers); }),
            nb::arg("state"),
            nb::arg("builder"),
            nb::arg("caches"),
            nb::arg("repository"),
            nb::arg("intermediates"),
            nb::arg("arguments"),
            nb::arg("registers"),
            nb::keep_alive<0, first_argument>(),
            nb::keep_alive<0, second_argument>(),
            nb::keep_alive<0, third_argument>(),
            nb::keep_alive<0, fourth_argument>(),
            nb::keep_alive<0, fifth_argument>(),
            nb::keep_alive<0, sixth_argument>(),
            nb::keep_alive<0, seventh_argument>());
    cls.def("get_state", &Context::get_state, nb::rv_policy::copy, nb::keep_alive<0, 1>());
}

inline void bind_evaluation_storage(nanobind::module_& m)
{
    namespace nb = nanobind;
    using Caches = runir::kr::dl::semantics::DenotationCaches;
    using Storage = runir::kr::dl::semantics::EvaluationStorage;
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
