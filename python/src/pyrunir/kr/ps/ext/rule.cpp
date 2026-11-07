#include "bindings.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>
#include "pyrunir/kr/binding_utils.hpp"

#include <nanobind/stl/list.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <yggdrasil/database/syntax/query_repository.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/rule_data.hpp>
#include <runir/kr/ps/ext/rule_view.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::ps::ext
{

namespace
{

template<typename T>
auto bind_rule_data(nb::module_& m, const char* name)
{
    using Data = urd::data<T>;
    auto cls =
        nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index).def_rw("source", &Data::source).def_rw("conditions", &Data::conditions);
    if constexpr (requires { &Data::target; })
        cls.def_rw("target", &Data::target);
    if constexpr (requires { &Data::effects; })
        cls.def_rw("effects", &Data::effects);
    if constexpr (requires { &Data::order; })
        cls.def_rw("order", &Data::order);
    urd::python::add_comparison(cls);
    return cls;
}

template<typename T>
auto bind_rule_view(nb::module_& m, const char* name)
{
    using View = urd::index_view<T, Repository>;
    auto cls = nb::class_<View>(m, name)
                   .def("index", &View::index)
                   .def("get_source", &View::get_source)
                   .def("get_conditions", &View::get_conditions);
    if constexpr (requires(View view) { view.get_target(); })
        cls.def("get_target", &View::get_target);
    if constexpr (requires { &View::get_effects; })
        cls.def("get_effects", &View::get_effects);
    if constexpr (requires(View view) { view.get_order(); })
        cls.def("get_order", &View::get_order);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
    return cls;
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

    using MemoryStateIndex = urd::index<MemoryState<ExtFamilyTag>>;
    using ConditionIndexList = runir::kr::ps::ConditionList<runir::kr::ExtFamilyTag>;
    using ConditionViewList = urd::Store<urd::sequence<urd::resolved_t<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>, Repository>>>;
    using EffectIndexList = runir::kr::ps::EffectList<runir::kr::ExtFamilyTag>;
    using EffectViewList = urd::Store<urd::sequence<urd::resolved_t<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>, Repository>>>;
    using ConceptFeatureIndex = urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
    using RoleFeatureIndex = urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
    using QueryFeatureIndex = urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>;
    using ConceptRegisterIndex = urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>;
    using RoleRegisterIndex = urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>;
    using OrderTermIndexList = urd::flat_vector<urd::index<OrderTerm>>;
    using OrderTermViewList = urd::Store<urd::sequence<urd::index_view<OrderTerm, Repository>>>;

    bind_rule_data<ConceptLoad>(m, "ConceptLoadRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, ConceptFeatureIndex, ConceptRegisterIndex, EffectIndexList>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def(urd::python::init_from_views<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>, Repository>,
                      runir::kr::dl::RegisterView<runir::kr::dl::ConceptTag>,
                      const EffectViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def_rw("feature", &urd::data<ConceptLoad>::feature)
        .def_rw("reg", &urd::data<ConceptLoad>::reg);
    bind_rule_data<RoleLoad>(m, "RoleLoadRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, RoleFeatureIndex, RoleRegisterIndex, EffectIndexList>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def(urd::python::init_from_views<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>, Repository>,
                      runir::kr::dl::RegisterView<runir::kr::dl::RoleTag>,
                      const EffectViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def_rw("feature", &urd::data<RoleLoad>::feature)
        .def_rw("reg", &urd::data<RoleLoad>::reg);
    bind_rule_data<ConceptChoose>(m, "ConceptChooseRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, ConceptFeatureIndex, ConceptRegisterIndex, EffectIndexList, OrderTermIndexList>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a,
             "order"_a)
        .def(urd::python::init_from_views<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>, Repository>,
                      runir::kr::dl::RegisterView<runir::kr::dl::ConceptTag>,
                      const EffectViewList&,
                      const OrderTermViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a,
             "order"_a)
        .def_rw("feature", &urd::data<ConceptChoose>::feature)
        .def_rw("reg", &urd::data<ConceptChoose>::reg);
    bind_rule_data<RoleChoose>(m, "RoleChooseRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, RoleFeatureIndex, RoleRegisterIndex, EffectIndexList, OrderTermIndexList>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a,
             "order"_a)
        .def(urd::python::init_from_views<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>, Repository>,
                      runir::kr::dl::RegisterView<runir::kr::dl::RoleTag>,
                      const EffectViewList&,
                      const OrderTermViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a,
             "order"_a)
        .def_rw("feature", &urd::data<RoleChoose>::feature)
        .def_rw("reg", &urd::data<RoleChoose>::reg);
    bind_rule_data<Sketch>(m, "SketchRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, EffectIndexList>(), "source"_a, "target"_a, "conditions"_a, "effects"_a)
        .def(urd::python::init_from_views<MemoryStateView, MemoryStateView, const ConditionViewList&, const EffectViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a);
    bind_rule_data<Action>(m, "ActionRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, EffectIndexList, urd::flat_string, QueryFeatureIndex>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "query_feature"_a)
        .def(urd::python::init_from_views<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      const EffectViewList&,
                      urd::flat_string,
                      urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>, Repository>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "query_feature"_a)
        .def_rw("action_name", &urd::data<Action>::action_name)
        .def_rw("query_feature", &urd::data<Action>::query_feature);
    bind_rule_data<Do>(m, "DoRuleData")
        .def(nb::init<MemoryStateIndex,
                      MemoryStateIndex,
                      ConditionIndexList,
                      EffectIndexList,
                      urd::flat_string,
                      urd::flat_vector<urd::index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "arguments"_a)
        .def(urd::python::init_from_views<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      const EffectViewList&,
                      urd::flat_string,
                      const urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>, Repository>>>&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "arguments"_a)
        .def_rw("action_name", &urd::data<Do>::action_name)
        .def_rw("arguments", &urd::data<Do>::arguments);
    bind_rule_data<Call>(m, "CallRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, urd::index<ModuleSymbol<ExtFamilyTag>>, urd::flat_vector<CallArgument>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "callee"_a,
             "arguments"_a)
        .def(urd::python::init_from_views<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      urd::index_view<ModuleSymbol<ExtFamilyTag>, Repository>,
                      urd::flat_vector<CallArgument>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "callee"_a,
             "arguments"_a)
        .def_rw("callee", &urd::data<Call>::callee)
        .def_rw("arguments", &urd::data<Call>::arguments);
    bind_rule_data<Backtrack>(m, "BacktrackRuleData")
        .def(nb::init<MemoryStateIndex, ConditionIndexList>(), "source"_a, "conditions"_a)
        .def(urd::python::init_from_views<MemoryStateView, const ConditionViewList&>(), "source"_a, "conditions"_a);

    bind_rule_view<ConceptLoad>(m, "ConceptLoadRule")
        .def("get_feature", &urd::index_view<ConceptLoad, Repository>::get_feature)
        .def("get_register", &urd::index_view<ConceptLoad, Repository>::get_register);
    bind_rule_view<RoleLoad>(m, "RoleLoadRule")
        .def("get_feature", &urd::index_view<RoleLoad, Repository>::get_feature)
        .def("get_register", &urd::index_view<RoleLoad, Repository>::get_register);
    bind_rule_view<ConceptChoose>(m, "ConceptChooseRule")
        .def("get_feature", &urd::index_view<ConceptChoose, Repository>::get_feature)
        .def("get_register", &urd::index_view<ConceptChoose, Repository>::get_register);
    bind_rule_view<RoleChoose>(m, "RoleChooseRule")
        .def("get_feature", &urd::index_view<RoleChoose, Repository>::get_feature)
        .def("get_register", &urd::index_view<RoleChoose, Repository>::get_register);
    bind_rule_view<Sketch>(m, "SketchRule");
    bind_rule_view<Action>(m, "ActionRule")
        .def("get_action_name", &urd::index_view<Action, Repository>::get_action_name)
        .def("get_query_feature", &urd::index_view<Action, Repository>::get_query_feature);
    bind_rule_view<Do>(m, "DoRule")
        .def("get_action_name", &urd::index_view<Do, Repository>::get_action_name)
        .def("get_action_arguments", &urd::index_view<Do, Repository>::get_action_arguments);
    using CallView = urd::index_view<Call, Repository>;
    bind_rule_view<Call>(m, "CallRule")
        .def("get_callee", &CallView::get_callee)
        .def("get_call_arguments", &CallView::get_call_arguments);
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
