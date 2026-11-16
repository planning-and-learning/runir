#include "bindings.hpp"
#include "pyrunir/module.hpp"
#include "runir/kr/ps/detail/repository_instantiations.hpp"

#include <nanobind/stl/list.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/rule_data.hpp>
#include <runir/kr/ps/ext/rule_view.hpp>
#include <tyr/datalog/lifted/database/syntax/query_repository.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::ext
{

void bind_rule(nb::module_& m, RepositoryBinding& repository)
{
    nb::enum_<OrderDirection>(m, "OrderDirection").value("MIN", OrderDirection::MIN).value("MAX", OrderDirection::MAX);
    urd::python::bind_data<OrderTerm>(m);
    urd::python::bind_view<OrderTermView>(m);
    ygg::bind_insert<OrderTerm>(repository);

    urd::python::bind_data<Rule<LoadTag<runir::kr::dl::ConceptTag>>>(m);
    urd::python::bind_data<Rule<LoadTag<runir::kr::dl::RoleTag>>>(m);
    urd::python::bind_data<Rule<ChooseTag<runir::kr::dl::ConceptTag>>>(m);
    urd::python::bind_data<Rule<ChooseTag<runir::kr::dl::RoleTag>>>(m);
    urd::python::bind_data<Rule<SketchTag>>(m);
    urd::python::bind_data<Rule<DoTag>>(m);
    urd::python::bind_data<Rule<ActionTag>>(m);
    urd::python::bind_data<Rule<CallTag>>(m);
    urd::python::bind_data<Rule<BacktrackTag>>(m);
    urd::python::bind_view<RuleView<LoadTag<runir::kr::dl::ConceptTag>>>(m);
    urd::python::bind_view<RuleView<LoadTag<runir::kr::dl::RoleTag>>>(m);
    urd::python::bind_view<RuleView<ChooseTag<runir::kr::dl::ConceptTag>>>(m);
    urd::python::bind_view<RuleView<ChooseTag<runir::kr::dl::RoleTag>>>(m);
    urd::python::bind_view<RuleView<SketchTag>>(m);
    urd::python::bind_view<RuleView<DoTag>>(m);
    urd::python::bind_view<RuleView<ActionTag>>(m);
    urd::python::bind_view<RuleView<CallTag>>(m);
    urd::python::bind_view<RuleView<BacktrackTag>>(m);
    ygg::bind_insert<Rule<LoadTag<runir::kr::dl::ConceptTag>>>(repository);
    ygg::bind_insert<Rule<LoadTag<runir::kr::dl::RoleTag>>>(repository);
    ygg::bind_insert<Rule<ChooseTag<runir::kr::dl::ConceptTag>>>(repository);
    ygg::bind_insert<Rule<ChooseTag<runir::kr::dl::RoleTag>>>(repository);
    ygg::bind_insert<Rule<SketchTag>>(repository);
    ygg::bind_insert<Rule<DoTag>>(repository);
    ygg::bind_insert<Rule<ActionTag>>(repository);
    ygg::bind_insert<Rule<CallTag>>(repository);
    ygg::bind_insert<Rule<BacktrackTag>>(repository);
}

}  // namespace runir::kr::ps::ext
