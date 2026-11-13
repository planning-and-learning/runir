#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/module.hpp"

#include <nanobind/stl/list.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/rule_data.hpp>
#include <runir/kr/ps/icp/rule_view.hpp>
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::icp
{

namespace
{

/// A rule's record: its fields (urd::Fields) as attributes and as the arguments of its __init__.
template<typename T>
void bind_rule_data(nb::module_& m, const char* name)
{
    using Data = urd::data<T>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index);
    runir::kr::python::def_fields<Repository>(cls);
    urd::python::add_comparison(cls);
}

/// A rule's view: get_<name> for each field of its record.
template<typename T>
void bind_rule_view(nb::module_& m, const char* name)
{
    using View = urd::index_view<T, Repository>;
    auto cls = nb::class_<View>(m, name).def("index", &View::index);
    urd::python::bind_field_getters(cls);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
}

}  // namespace

void bind_rule(nb::module_& m, RepositoryBinding& repository)
{
    using namespace nb::literals;
    using ConceptLoad = Rule<LoadTag<runir::kr::dl::ConceptTag>>;
    using RoleLoad = Rule<LoadTag<runir::kr::dl::RoleTag>>;
    using Crule = Rule<CruleTag>;

    urd::python::bind_index<urd::index<ConceptLoad>>(m, "ConceptLoadRuleIndex");
    urd::python::bind_index<urd::index<RoleLoad>>(m, "RoleLoadRuleIndex");
    urd::python::bind_index<urd::index<Crule>>(m, "CruleRuleIndex");
    bind_rule_data<ConceptLoad>(m, "ConceptLoadRuleData");
    bind_rule_data<RoleLoad>(m, "RoleLoadRuleData");
    bind_rule_data<Crule>(m, "CruleRuleData");

    bind_rule_view<ConceptLoad>(m, "ConceptLoadRule");
    bind_rule_view<RoleLoad>(m, "RoleLoadRule");
    bind_rule_view<Crule>(m, "CruleRule");
    ygg::bind_insert<ConceptLoad>(repository);
    ygg::bind_insert<RoleLoad>(repository);
    ygg::bind_insert<Crule>(repository);
}

}  // namespace runir::kr::ps::icp
