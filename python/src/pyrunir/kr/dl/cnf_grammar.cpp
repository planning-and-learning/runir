#include "dl.hpp"

#include "binding_helpers.hpp"

#include <memory>
#include <nanobind/stl/chrono.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/vector.h>
#include <runir/knowledge_representation/dl/cnf_grammar/datas.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/formatter.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/generate.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/translate.hpp>
#include <runir/knowledge_representation/dl/cnf_grammar/views.hpp>
#include <runir/knowledge_representation/dl/repository.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/ground_task/state_view.hpp>
#include <tyr/planning/lifted_task/state_view.hpp>

namespace runir::kr::dl
{

namespace cnf = runir::kr::dl::cnf_grammar;
namespace fp = tyr::formalism::planning;

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

void bind_cnf_grammar_module_definitions(nb::module_& m)
{
    python_detail::bind_constructor_indices<cnf::Concept, cnf::Role, cnf::Boolean, cnf::Numerical, cnf::Constructor>(m);
    python_detail::bind_constructor_datas<cnf::Concept, cnf::Role, cnf::Boolean, cnf::Numerical, cnf::Constructor>(m);
    python_detail::bind_constructor_views<cnf::Concept, cnf::Role, cnf::Boolean, cnf::Numerical, cnf::Constructor, cnf::ConstructorRepository>(m);

    auto repository = nb::class_<cnf::ConstructorRepository>(m, "ConstructorRepository");
    repository.def(nb::new_([](size_t index, std::shared_ptr<const fp::Repository> planning_repository)
                            { return std::make_unique<cnf::ConstructorRepository>(index, std::move(planning_repository)); }),
                   "index"_a,
                   "planning_repository"_a);
    python_detail::bind_clearable_repository(repository);

    python_detail::bind_view<cnf::GrammarTag, cnf::ConstructorRepository>(m, "Grammar");

    m.def("translate", &cnf::translate, "grammar"_a, "repository"_a, nb::keep_alive<0, 2>());

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
