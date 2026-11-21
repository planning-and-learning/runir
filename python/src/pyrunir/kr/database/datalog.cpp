#include "bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/database/datalog/construction.hpp"
#include "runir/kr/database/datalog/evaluator.hpp"
#include "yggdrasil/python/bindings.hpp"

#include <nanobind/stl/pair.h>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>

namespace runir::kr::database
{
namespace
{
template<typename T>
using Sequence = urd::Store<urd::sequence<T>>;

template<typename Input>
void bind_inputs(nb::class_<datalog::Evaluator>& cls)
{
    using Inputs = Sequence<Input>;
    using Changes = Sequence<std::pair<Input, Input>>;
    cls.def("evaluate", &datalog::Evaluator::template evaluate<Inputs>, nb::arg("inputs"))
        .def("initialize", &datalog::Evaluator::template initialize<Inputs>, nb::arg("inputs"))
        .def("update", &datalog::Evaluator::template update<Changes>, nb::arg("changes"), "One exact (added, removed) pair per input slot.");
}

/// A Datalog record: its index, its record (Python builds it through its empty __init__ and its fields), its builder,
/// its view and DatalogRepository.insert of the record, prepared first (prepare_for_insert).
template<typename T>
void bind_record(nb::module_& m, nb::class_<datalog::DatalogRepository>& repository)
{
    urd::python::bind_index<T>(m);
    urd::python::bind_data<T>(m);
    urd::python::bind_builder<T, datalog::DatalogRepository>(m);
    urd::python::bind_view<urd::index_view<T, datalog::DatalogRepository>>(m);
    ygg::bind_insert<T>(repository);
}
}  // namespace

void bind_datalog(nb::module_& m)
{
    using namespace nb::literals;

    nb::enum_<datalog::AggregateFunction>(m, "DatalogAggregateFunction")
        .value("COUNT", datalog::AggregateFunction::COUNT)
        .value("SUM", datalog::AggregateFunction::SUM)
        .value("MIN", datalog::AggregateFunction::MIN)
        .value("MAX", datalog::AggregateFunction::MAX);

    auto term = nb::class_<datalog::Term>(m, "DatalogTerm", "A variable, numbered within its rule, or an object constant.");
    term.def_static("variable", &datalog::Term::variable, "variable"_a)
        .def_static("constant", &datalog::Term::constant, "object"_a)
        .def("is_variable", &datalog::Term::is_variable)
        .def_ro("value", &datalog::Term::value)
        .def("__eq__", [](const datalog::Term& lhs, const datalog::Term& rhs) { return lhs == rhs; }, nb::is_operator());

    auto comparison = nb::class_<datalog::Comparison>(m, "DatalogComparison", "lhs kind rhs, ordered as uint_t (the largest value is infinity).");
    nb::enum_<datalog::Comparison::Kind>(m, "DatalogComparisonKind")
        .value("EQUAL", datalog::Comparison::Kind::EQUAL)
        .value("NOT_EQUAL", datalog::Comparison::Kind::NOT_EQUAL)
        .value("LESS", datalog::Comparison::Kind::LESS)
        .value("LESS_EQUAL", datalog::Comparison::Kind::LESS_EQUAL);
    comparison
        .def(
            "__init__",
            [](datalog::Comparison* self, datalog::Comparison::Kind kind, datalog::Term lhs, datalog::Term rhs)
            { new (self) datalog::Comparison { kind, lhs, rhs }; },
            "kind"_a,
            "lhs"_a,
            "rhs"_a)
        .def_ro("kind", &datalog::Comparison::kind)
        .def_ro("lhs", &datalog::Comparison::lhs)
        .def_ro("rhs", &datalog::Comparison::rhs);

    auto arithmetic =
        nb::class_<datalog::Arithmetic>(m, "DatalogArithmetic", "result = lhs operation rhs over saturating numbers; the largest value is infinity.");
    nb::enum_<datalog::Arithmetic::Operation>(m, "DatalogArithmeticOperation")
        .value("ADD", datalog::Arithmetic::Operation::ADD)
        .value("SUBTRACT", datalog::Arithmetic::Operation::SUBTRACT)
        .value("MULTIPLY", datalog::Arithmetic::Operation::MULTIPLY)
        .value("DIVIDE", datalog::Arithmetic::Operation::DIVIDE)
        .value("MIN", datalog::Arithmetic::Operation::MIN)
        .value("MAX", datalog::Arithmetic::Operation::MAX);
    arithmetic
        .def(
            "__init__",
            [](datalog::Arithmetic* self, datalog::Arithmetic::Operation operation, datalog::Term lhs, datalog::Term rhs, ygg::uint_t result)
            { new (self) datalog::Arithmetic { operation, lhs, rhs, result }; },
            "operation"_a,
            "lhs"_a,
            "rhs"_a,
            "result"_a)
        .def_ro("operation", &datalog::Arithmetic::operation)
        .def_ro("lhs", &datalog::Arithmetic::lhs)
        .def_ro("rhs", &datalog::Arithmetic::rhs)
        .def_ro("result", &datalog::Arithmetic::result);

    auto repository =
        nb::class_<datalog::DatalogRepository>(m, "DatalogRepository", "Interns validated Datalog relations, atoms, aggregates, rules and programs.");
    repository.def(nb::init<>());
    bind_record<datalog::Relation>(m, repository);
    bind_record<datalog::Atom>(m, repository);
    bind_record<datalog::Aggregate>(m, repository);
    bind_record<datalog::Rule>(m, repository);
    bind_record<datalog::Program>(m, repository);

    {
        using E = datalog::Evaluator;
        auto cls = nb::class_<E>(m,
                                 "DatalogEvaluator",
                                 "Evaluate a program's relations rule by rule (semi-naive), and maintain them under input changes (counting, "
                                 "Delete and Rederive). Its results are drawn from the scratch relations. Static input slots are read once.");
        cls.def(
               "__init__",
               [](E* self,
                  ygg::FreeScratchRelations& scratch,
                  datalog::ProgramView program,
                  const Sequence<datalog::RelationView>& outputs,
                  const Sequence<size_t>& static_slots) { new (self) E(scratch, program, outputs, static_slots); },
               "scratch"_a,
               "program"_a,
               "outputs"_a,
               "static_slots"_a = Sequence<size_t> {},
               nb::keep_alive<1, 2>(),
               nb::keep_alive<1, 3>())
            .def(
                "get_result",
                [](const E& evaluator, size_t output) { return evaluator.get_result(output); },
                "output"_a = 0,
                nb::keep_alive<0, 1>(),
                "Borrowed result; the next evaluate, initialize or update invalidates it.")
            .def(
                "get_delta",
                [](const E& evaluator, size_t output) -> const incremental::Delta& { return evaluator.get_delta(output); },
                "output"_a = 0,
                nb::rv_policy::reference_internal)
            .def("memory_usage", &E::memory_usage);
        bind_inputs<ygg::FreeRelationBuilderView>(cls);
        bind_inputs<ygg::FreeRelationView>(cls);
    }
}
}  // namespace runir::kr::database
