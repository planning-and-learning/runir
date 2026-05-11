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

namespace cnf = runir::kr::dl::cnf_grammar;

using namespace nanobind::literals;

namespace
{

template<tyr::planning::TaskKind Kind>
auto generate(cnf::GrammarView grammar,
              const std::vector<tyr::planning::StateView<Kind>>& states,
              runir::kr::dl::ConstructorRepository& output_repository,
              const cnf::GenerateOptions& options)
{
    return cnf::generate<Kind>(grammar, states, output_repository, options);
}

}  // namespace

void bind_cnf_grammar_generate(nb::module_& m)
{
    nb::class_<cnf::GenerateStatistics>(m, "GenerateStatistics")
        .def_ro("num_generated", &cnf::GenerateStatistics::num_generated)
        .def_ro("num_pruned", &cnf::GenerateStatistics::num_pruned)
        .def_ro("num_kept", &cnf::GenerateStatistics::num_kept)
        .def_ro("total_time", &cnf::GenerateStatistics::total_time);

    nb::class_<cnf::GenerateOptions>(m, "GenerateOptions")
        .def(nb::init<>())
        .def_rw("max_syntactic_complexity", &cnf::GenerateOptions::max_syntactic_complexity)
        .def_rw("max_time", &cnf::GenerateOptions::max_time);

    nb::enum_<cnf::GenerateStatus>(m, "GenerateStatus")
        .value("COMPLETED", cnf::GenerateStatus::COMPLETED)
        .value("OUT_OF_TIME", cnf::GenerateStatus::OUT_OF_TIME);

    nb::class_<cnf::GenerateResults>(m, "GenerateResults")
        .def_ro("statistics", &cnf::GenerateResults::statistics)
        .def_ro("status", &cnf::GenerateResults::status)
        .def_ro("concepts", &cnf::GenerateResults::concepts)
        .def_ro("roles", &cnf::GenerateResults::roles)
        .def_ro("booleans", &cnf::GenerateResults::booleans)
        .def_ro("numericals", &cnf::GenerateResults::numericals);

    m.def("generate_ground", &generate<tyr::planning::GroundTag>, "grammar"_a, "states"_a, "output_repository"_a, "options"_a, nb::keep_alive<0, 3>());
    m.def("generate_lifted", &generate<tyr::planning::LiftedTag>, "grammar"_a, "states"_a, "output_repository"_a, "options"_a, nb::keep_alive<0, 3>());
}

}  // namespace runir::kr::dl
