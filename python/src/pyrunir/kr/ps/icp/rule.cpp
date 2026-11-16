#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/list.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/rule_data.hpp>
#include <runir/kr/ps/icp/rule_view.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::icp
{

void bind_rule(nb::module_& m, RepositoryBinding& repository)
{
    urd::python::bind_data<Rule<LoadTag<runir::kr::dl::ConceptTag>>>(m);
    urd::python::bind_data<Rule<LoadTag<runir::kr::dl::RoleTag>>>(m);
    urd::python::bind_data<Rule<CruleTag>>(m);
    urd::python::bind_view<RuleView<LoadTag<runir::kr::dl::ConceptTag>>>(m);
    urd::python::bind_view<RuleView<LoadTag<runir::kr::dl::RoleTag>>>(m);
    urd::python::bind_view<RuleView<CruleTag>>(m);
    ygg::bind_insert<Rule<LoadTag<runir::kr::dl::ConceptTag>>>(repository);
    ygg::bind_insert<Rule<LoadTag<runir::kr::dl::RoleTag>>>(repository);
    ygg::bind_insert<Rule<CruleTag>>(repository);
}

}  // namespace runir::kr::ps::icp
