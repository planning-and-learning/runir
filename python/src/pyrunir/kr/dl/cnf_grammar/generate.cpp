#include "module.hpp"

#include <nanobind/stl/chrono.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/cnf_grammar/generate.hpp>
#include <runir/kr/dl/repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>

namespace runir::kr::dl
{

namespace
{

template<tyr::planning::TaskKind Kind>
auto generate(runir::kr::dl::cnf_grammar::GrammarView grammar,
              const std::vector<tyr::planning::StateView<Kind>>& states,
              runir::kr::dl::ConstructorRepository& output_repository,
              const runir::kr::dl::cnf_grammar::GenerateOptions& options)
{
    return runir::kr::dl::cnf_grammar::generate<Kind>(grammar, states, output_repository, options);
}

}  // namespace

void bind_cnf_grammar_generate(nb::module_& m)
{
    nb::class_<runir::kr::dl::cnf_grammar::GenerateStatistics>(m, "GenerateStatistics")
        .def_ro("num_generated", &runir::kr::dl::cnf_grammar::GenerateStatistics::num_generated)
        .def_ro("num_pruned", &runir::kr::dl::cnf_grammar::GenerateStatistics::num_pruned)
        .def_ro("num_kept", &runir::kr::dl::cnf_grammar::GenerateStatistics::num_kept)
        .def_ro("total_time", &runir::kr::dl::cnf_grammar::GenerateStatistics::total_time);

    nb::class_<runir::kr::dl::cnf_grammar::GenerateOptions>(m, "GenerateOptions")
        .def(nb::init<>())
        .def_rw("max_syntactic_complexity", &runir::kr::dl::cnf_grammar::GenerateOptions::max_syntactic_complexity)
        .def_rw("max_time", &runir::kr::dl::cnf_grammar::GenerateOptions::max_time);

    nb::enum_<runir::kr::dl::cnf_grammar::GenerateStatus>(m, "GenerateStatus")
        .value("COMPLETED", runir::kr::dl::cnf_grammar::GenerateStatus::COMPLETED)
        .value("OUT_OF_TIME", runir::kr::dl::cnf_grammar::GenerateStatus::OUT_OF_TIME);

    nb::class_<runir::kr::dl::cnf_grammar::GenerateResults>(m, "GenerateResults")
        .def_ro("statistics", &runir::kr::dl::cnf_grammar::GenerateResults::statistics)
        .def_ro("status", &runir::kr::dl::cnf_grammar::GenerateResults::status)
        .def_ro("concepts", &runir::kr::dl::cnf_grammar::GenerateResults::concepts)
        .def_ro("roles", &runir::kr::dl::cnf_grammar::GenerateResults::roles)
        .def_ro("booleans", &runir::kr::dl::cnf_grammar::GenerateResults::booleans)
        .def_ro("numericals", &runir::kr::dl::cnf_grammar::GenerateResults::numericals);

    m.def("generate_ground",
          &generate<tyr::planning::GroundTag>,
          nb::arg("grammar"),
          nb::arg("states"),
          nb::arg("output_repository"),
          nb::arg("options"),
          nb::keep_alive<0, 3>());
    m.def("generate_lifted",
          &generate<tyr::planning::LiftedTag>,
          nb::arg("grammar"),
          nb::arg("states"),
          nb::arg("output_repository"),
          nb::arg("options"),
          nb::keep_alive<0, 3>());
}

}  // namespace runir::kr::dl
