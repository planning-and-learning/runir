#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <nanobind/stl/list.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/rule_data.hpp>
#include <runir/kr/ps/ext/rule_view.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext
{

using namespace nanobind::literals;

namespace
{

template<typename T>
auto bind_rule_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<T>;
    auto cls = nb::class_<Data>(m, name)
                   .def(nb::init<>())
                   .def_rw("index", &Data::index)
                   .def_rw("source", &Data::source)
                   .def_rw("conditions", &Data::conditions);
    if constexpr (requires { &Data::target; })
        cls.def_rw("target", &Data::target);
    if constexpr (requires { &Data::effects; })
        cls.def_rw("effects", &Data::effects);
    if constexpr (requires { &Data::order; })
        cls.def_rw("order", &Data::order);
    ygg::add_comparison(cls);
    return cls;
}

template<typename T>
auto bind_rule_view(nb::module_& m, const char* name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    auto cls = nb::class_<View>(m, name)
                   .def("get_index", &View::get_index)
                   .def("get_source", &View::get_source, nb::keep_alive<0, 1>())
                   .def("get_conditions", &View::get_conditions);
    if constexpr (requires(View view) { view.get_target(); })
        cls.def("get_target", &View::get_target, nb::keep_alive<0, 1>());
    if constexpr (requires { &View::get_effects; })
        cls.def("get_effects", &View::get_effects);
    if constexpr (requires(View view) { view.get_order(); })
        cls.def("get_order", &View::get_order);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    return cls;
}

}  // namespace

void bind_rule(nb::module_& m, RepositoryBinding& repository)
{
    nb::enum_<OrderDirection>(m, "OrderDirection").value("MIN", OrderDirection::MIN).value("MAX", OrderDirection::MAX);
    using TermData = ygg::Data<OrderTerm>;
    ygg::bind_index<ygg::Index<OrderTerm>>(m, "OrderTermIndex");
    auto term = nb::class_<TermData>(m, "OrderTermData")
                    .def(nb::init<>())
                    .def(nb::init<OrderDirection, TermData::Feature>(), "direction"_a, "feature"_a)
                    .def(nb::init<OrderDirection, TermData::ViewVariant<Repository>>(), "direction"_a, "feature"_a)
                    .def_rw("index", &TermData::index)
                    .def_rw("direction", &TermData::direction)
                    .def_rw("feature", &TermData::feature);
    ygg::add_comparison(term);
    using TermView = OrderTermView;
    auto term_view = nb::class_<TermView>(m, "OrderTerm")
                         .def("get_index", &TermView::get_index)
                         .def("get_direction", &TermView::get_direction)
                         .def("get_feature", &TermView::get_feature, nb::keep_alive<0, 1>());
    ygg::add_comparison(term_view);
    ygg::add_hash(term_view);
    runir::kr::python::bind_insert<OrderTerm>(repository);

    using ConceptLoad = Rule<LoadTag<runir::kr::dl::ConceptTag>>;
    using RoleLoad = Rule<LoadTag<runir::kr::dl::RoleTag>>;
    using ConceptChoose = Rule<ChooseTag<runir::kr::dl::ConceptTag>>;
    using RoleChoose = Rule<ChooseTag<runir::kr::dl::RoleTag>>;
    using Sketch = Rule<SketchTag>;
    using Do = Rule<DoTag>;
    using Action = Rule<ActionTag>;
    using Call = Rule<CallTag>;
    using Backtrack = Rule<BacktrackTag>;

    ygg::bind_index<ygg::Index<ConceptLoad>>(m, "ConceptLoadRuleIndex");
    ygg::bind_index<ygg::Index<RoleLoad>>(m, "RoleLoadRuleIndex");
    ygg::bind_index<ygg::Index<ConceptChoose>>(m, "ConceptChooseRuleIndex");
    ygg::bind_index<ygg::Index<RoleChoose>>(m, "RoleChooseRuleIndex");
    ygg::bind_index<ygg::Index<Sketch>>(m, "SketchRuleIndex");
    ygg::bind_index<ygg::Index<Do>>(m, "DoRuleIndex");
    ygg::bind_index<ygg::Index<Action>>(m, "ActionRuleIndex");
    ygg::bind_index<ygg::Index<Call>>(m, "CallRuleIndex");
    ygg::bind_index<ygg::Index<Backtrack>>(m, "BacktrackRuleIndex");

    using MemoryStateIndex = ygg::Index<MemoryState>;
    using MemoryStateView = ygg::View<ygg::Index<MemoryState>, Repository>;
    using ConditionIndexList = ygg::IndexList<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>;
    using ConditionViewList = std::vector<ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<runir::kr::ExtFamilyTag>>, Repository>>;
    using EffectIndexList = ygg::IndexList<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>;
    using EffectViewList = std::vector<ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::ExtFamilyTag>>, Repository>>;
    using ConceptFeatureIndex = ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>;
    using RoleFeatureIndex = ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>>;
    using QueryFeatureIndex = ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::QueryFeature>>;
    using ConceptRegisterIndex = ygg::Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>;
    using RoleRegisterIndex = ygg::Index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>;
    using OrderTermIndexList = ygg::IndexList<OrderTerm>;
    using OrderTermViewList = std::vector<ygg::View<ygg::Index<OrderTerm>, Repository>>;

    bind_rule_data<ConceptLoad>(m, "ConceptLoadRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, ConceptFeatureIndex, ConceptRegisterIndex, EffectIndexList>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def(nb::init<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      ygg::View<ConceptFeatureIndex, Repository>,
                      runir::kr::dl::RegisterView<runir::kr::dl::ConceptTag>,
                      const EffectViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def_rw("feature", &ygg::Data<ConceptLoad>::feature)
        .def_rw("reg", &ygg::Data<ConceptLoad>::reg);
    bind_rule_data<RoleLoad>(m, "RoleLoadRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, RoleFeatureIndex, RoleRegisterIndex, EffectIndexList>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def(nb::init<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      ygg::View<RoleFeatureIndex, Repository>,
                      runir::kr::dl::RegisterView<runir::kr::dl::RoleTag>,
                      const EffectViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def_rw("feature", &ygg::Data<RoleLoad>::feature)
        .def_rw("reg", &ygg::Data<RoleLoad>::reg);
    bind_rule_data<ConceptChoose>(m, "ConceptChooseRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, ConceptFeatureIndex, ConceptRegisterIndex, EffectIndexList, OrderTermIndexList>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a,
             "order"_a)
        .def(nb::init<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      ygg::View<ConceptFeatureIndex, Repository>,
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
        .def_rw("feature", &ygg::Data<ConceptChoose>::feature)
        .def_rw("reg", &ygg::Data<ConceptChoose>::reg);
    bind_rule_data<RoleChoose>(m, "RoleChooseRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, RoleFeatureIndex, RoleRegisterIndex, EffectIndexList, OrderTermIndexList>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a,
             "order"_a)
        .def(nb::init<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      ygg::View<RoleFeatureIndex, Repository>,
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
        .def_rw("feature", &ygg::Data<RoleChoose>::feature)
        .def_rw("reg", &ygg::Data<RoleChoose>::reg);
    bind_rule_data<Sketch>(m, "SketchRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, EffectIndexList>(), "source"_a, "target"_a, "conditions"_a, "effects"_a)
        .def(nb::init<MemoryStateView, MemoryStateView, const ConditionViewList&, const EffectViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a);
    bind_rule_data<Action>(m, "ActionRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, EffectIndexList, ::cista::offset::string, QueryFeatureIndex>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "query_feature"_a)
        .def(nb::init<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      const EffectViewList&,
                      ::cista::offset::string,
                      ygg::View<QueryFeatureIndex, Repository>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "query_feature"_a)
        .def_rw("action_name", &ygg::Data<Action>::action_name)
        .def_rw("query_feature", &ygg::Data<Action>::query_feature);
    bind_rule_data<Do>(m, "DoRuleData")
        .def(nb::init<MemoryStateIndex,
                      MemoryStateIndex,
                      ConditionIndexList,
                      EffectIndexList,
                      ::cista::offset::string,
                      ygg::IndexList<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "arguments"_a)
        .def(nb::init<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      const EffectViewList&,
                      ::cista::offset::string,
                      const std::vector<ygg::View<ConceptFeatureIndex, Repository>>&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "arguments"_a)
        .def_rw("action_name", &ygg::Data<Do>::action_name)
        .def_rw("arguments", &ygg::Data<Do>::arguments);
    bind_rule_data<Call>(m, "CallRuleData")
        .def(nb::init<MemoryStateIndex, MemoryStateIndex, ConditionIndexList, ygg::Index<ModuleSymbol>, ::cista::offset::vector<CallArgument>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "callee"_a,
             "arguments"_a)
        .def(nb::init<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      ygg::View<ygg::Index<ModuleSymbol>, Repository>,
                      ::cista::offset::vector<CallArgument>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "callee"_a,
             "arguments"_a)
        .def_rw("callee", &ygg::Data<Call>::callee)
        .def_rw("arguments", &ygg::Data<Call>::arguments);
    bind_rule_data<Backtrack>(m, "BacktrackRuleData")
        .def(nb::init<MemoryStateIndex, ConditionIndexList>(), "source"_a, "conditions"_a)
        .def(nb::init<MemoryStateView, const ConditionViewList&>(), "source"_a, "conditions"_a);

    bind_rule_view<ConceptLoad>(m, "ConceptLoadRule")
        .def("get_feature", &ygg::View<ygg::Index<ConceptLoad>, Repository>::get_feature, nb::keep_alive<0, 1>())
        .def("get_register", &ygg::View<ygg::Index<ConceptLoad>, Repository>::get_register, nb::keep_alive<0, 1>());
    bind_rule_view<RoleLoad>(m, "RoleLoadRule")
        .def("get_feature", &ygg::View<ygg::Index<RoleLoad>, Repository>::get_feature, nb::keep_alive<0, 1>())
        .def("get_register", &ygg::View<ygg::Index<RoleLoad>, Repository>::get_register, nb::keep_alive<0, 1>());
    bind_rule_view<ConceptChoose>(m, "ConceptChooseRule")
        .def("get_feature", &ygg::View<ygg::Index<ConceptChoose>, Repository>::get_feature, nb::keep_alive<0, 1>())
        .def("get_register", &ygg::View<ygg::Index<ConceptChoose>, Repository>::get_register, nb::keep_alive<0, 1>());
    bind_rule_view<RoleChoose>(m, "RoleChooseRule")
        .def("get_feature", &ygg::View<ygg::Index<RoleChoose>, Repository>::get_feature, nb::keep_alive<0, 1>())
        .def("get_register", &ygg::View<ygg::Index<RoleChoose>, Repository>::get_register, nb::keep_alive<0, 1>());
    bind_rule_view<Sketch>(m, "SketchRule");
    bind_rule_view<Action>(m, "ActionRule")
        .def("get_action_name", &ygg::View<ygg::Index<Action>, Repository>::get_action_name)
        .def("get_query_feature", &ygg::View<ygg::Index<Action>, Repository>::get_query_feature, nb::keep_alive<0, 1>());
    bind_rule_view<Do>(m, "DoRule")
        .def("get_action_name", &ygg::View<ygg::Index<Do>, Repository>::get_action_name)
        .def("get_action_arguments", &ygg::View<ygg::Index<Do>, Repository>::get_action_arguments);
    using CallView = ygg::View<ygg::Index<Call>, Repository>;
    bind_rule_view<Call>(m, "CallRule")
        .def("get_callee", &CallView::get_callee, nb::keep_alive<0, 1>())
        .def("get_call_arguments", &CallView::get_call_arguments);
    bind_rule_view<Backtrack>(m, "BacktrackRule");

    runir::kr::python::bind_insert<ConceptLoad>(repository);
    runir::kr::python::bind_insert<RoleLoad>(repository);
    runir::kr::python::bind_insert<ConceptChoose>(repository);
    runir::kr::python::bind_insert<RoleChoose>(repository);
    runir::kr::python::bind_insert<Sketch>(repository);
    runir::kr::python::bind_insert<Do>(repository);
    runir::kr::python::bind_insert<Action>(repository);
    runir::kr::python::bind_insert<Call>(repository);
    runir::kr::python::bind_insert<Backtrack>(repository);
}

}  // namespace runir::kr::ps::ext
