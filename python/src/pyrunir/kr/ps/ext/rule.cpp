#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"
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
#include <urd/python/sequences/sequence.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::ext
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
    nb::enum_<OrderDirection>(m, "OrderDirection").value("MIN", OrderDirection::MIN).value("MAX", OrderDirection::MAX);
    using TermData = urd::data<OrderTerm>;
    urd::python::bind_index<urd::index<OrderTerm>>(m, "OrderTermIndex");
    auto term = nb::class_<TermData>(m, "OrderTermData")
                    .def(nb::init<>())
                    .def(nb::init<OrderDirection, TermData::Feature>(), "direction"_a, "feature"_a)
                    .def(urd::python::init_from_views<OrderDirection, urd::resolved_t<TermData::Feature, Repository>>(), "direction"_a, "feature"_a)
                    .def_rw("index", &TermData::index)
                    .def_rw("direction", &TermData::direction)
                    .def_rw("feature", &TermData::feature);
    urd::python::add_comparison(term);
    using TermView = OrderTermView;
    auto term_view = nb::class_<TermView>(m, "OrderTerm")
                         .def("index", &TermView::index)
                         .def("get_direction", &TermView::get_direction)
                         .def("get_feature", &TermView::get_feature);
    urd::python::add_comparison(term_view);
    urd::python::add_hash(term_view);
    ygg::bind_insert<OrderTerm>(repository);

    using ConceptLoad = Rule<LoadTag<runir::kr::dl::ConceptTag>>;
    using RoleLoad = Rule<LoadTag<runir::kr::dl::RoleTag>>;
    using ConceptChoose = Rule<ChooseTag<runir::kr::dl::ConceptTag>>;
    using RoleChoose = Rule<ChooseTag<runir::kr::dl::RoleTag>>;
    using Sketch = Rule<SketchTag>;
    using Do = Rule<DoTag>;
    using Action = Rule<ActionTag>;
    using Call = Rule<CallTag>;
    using Backtrack = Rule<BacktrackTag>;

    urd::python::bind_index<urd::index<ConceptLoad>>(m, "ConceptLoadRuleIndex");
    urd::python::bind_index<urd::index<RoleLoad>>(m, "RoleLoadRuleIndex");
    urd::python::bind_index<urd::index<ConceptChoose>>(m, "ConceptChooseRuleIndex");
    urd::python::bind_index<urd::index<RoleChoose>>(m, "RoleChooseRuleIndex");
    urd::python::bind_index<urd::index<Sketch>>(m, "SketchRuleIndex");
    urd::python::bind_index<urd::index<Do>>(m, "DoRuleIndex");
    urd::python::bind_index<urd::index<Action>>(m, "ActionRuleIndex");
    urd::python::bind_index<urd::index<Call>>(m, "CallRuleIndex");
    urd::python::bind_index<urd::index<Backtrack>>(m, "BacktrackRuleIndex");

    bind_rule_data<ConceptLoad>(m, "ConceptLoadRuleData");
    bind_rule_data<RoleLoad>(m, "RoleLoadRuleData");
    bind_rule_data<ConceptChoose>(m, "ConceptChooseRuleData");
    bind_rule_data<RoleChoose>(m, "RoleChooseRuleData");
    bind_rule_data<Sketch>(m, "SketchRuleData");
    bind_rule_data<Action>(m, "ActionRuleData");
    bind_rule_data<Do>(m, "DoRuleData");
    bind_rule_data<Call>(m, "CallRuleData");
    bind_rule_data<Backtrack>(m, "BacktrackRuleData");

    bind_rule_view<ConceptLoad>(m, "ConceptLoadRule");
    bind_rule_view<RoleLoad>(m, "RoleLoadRule");
    bind_rule_view<ConceptChoose>(m, "ConceptChooseRule");
    bind_rule_view<RoleChoose>(m, "RoleChooseRule");
    bind_rule_view<Sketch>(m, "SketchRule");
    bind_rule_view<Action>(m, "ActionRule");
    bind_rule_view<Do>(m, "DoRule");
    bind_rule_view<Call>(m, "CallRule");
    bind_rule_view<Backtrack>(m, "BacktrackRule");

    ygg::bind_insert<ConceptLoad>(repository);
    ygg::bind_insert<RoleLoad>(repository);
    ygg::bind_insert<ConceptChoose>(repository);
    ygg::bind_insert<RoleChoose>(repository);
    ygg::bind_insert<Sketch>(repository);
    ygg::bind_insert<Do>(repository);
    ygg::bind_insert<Action>(repository);
    ygg::bind_insert<Call>(repository);
    ygg::bind_insert<Backtrack>(repository);
}

}  // namespace runir::kr::ps::ext
