#ifndef PYRUNIR_KR_DL_EVALUATION_BINDINGS_HPP_
#define PYRUNIR_KR_DL_EVALUATION_BINDINGS_HPP_

#include <nanobind/nanobind.h>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>

namespace runir::kr::python
{
template<runir::kr::FamilyTag Family>
void bind_evaluation_storage(nanobind::module_& m)
{
    namespace nb = nanobind;
    namespace sem = runir::kr::dl::semantics;
    using Caches = sem::DenotationCaches<Family>;
    using Storage = sem::EvaluationStorage<Family>;
    nb::class_<Caches>(m, "DenotationCaches", "Memoized interned views; does not own result storage.")
        .def(nb::init<>())
        .def("reset_dynamic", &Caches::reset_dynamic)
        .def("reset_all", &Caches::reset_all);
    nb::class_<Storage>(m, "EvaluationStorage", "Reusable result repositories and matching memoization.")
        .def(nb::init<const sem::DenotationRepository&>(), nb::arg("prototype"))
        .def(
            "get_caches",
            [](Storage& self) -> Caches& { return self.get_caches(); },
            nb::rv_policy::reference_internal)
        .def("reset_dynamic", &Storage::reset_dynamic)
        .def("reset_all", &Storage::reset_all);
}
}

#endif
