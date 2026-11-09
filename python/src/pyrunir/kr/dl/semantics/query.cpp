#include "../query_bindings.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "pyrunir/module.hpp"
#include "bindings.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>

namespace runir::kr::dl
{

void bind_semantics_query(nb::module_& m)
{
    urd::python::bind_index<urd::index<QueryColumn>>(m, "QueryColumnIndex");
    auto data = nb::class_<urd::data<QueryColumn>>(m, "QueryColumnData")
                    .def(nb::init<>())
                    .def(nb::init<urd::flat_string>(), nb::arg("name"))
                    .def_rw("index", &urd::data<QueryColumn>::index)
                    .def_rw("name", &urd::data<QueryColumn>::name);
    urd::python::add_comparison(data);
    python::bind_queries(m);
}

}  // namespace runir::kr::dl
