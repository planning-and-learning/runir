#include "bindings.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <urd/python/sequences/sequence.hpp>
#include "pyrunir/kr/binding_utils.hpp"

#include <nanobind/stl/list.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/rule_data.hpp>
#include <runir/kr/ps/icp/rule_view.hpp>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include "pyrunir/module.hpp"

namespace runir::kr::ps::icp
{

namespace
{

template<typename T>
auto bind_rule_data(nb::module_& m, const char* name)
{
    using Data = urd::data<T>;
    auto cls = nb::class_<Data>(m, name)
                   .def(nb::init<>())
                   .def_rw("index", &Data::index)
                   .def_rw("source", &Data::source)
                   .def_rw("target", &Data::target)
                   .def_rw("conditions", &Data::conditions);
    if constexpr (requires { &Data::effects; })
        cls.def_rw("effects", &Data::effects);
    urd::python::add_comparison(cls);
    return cls;
}

template<typename T>
auto bind_rule_view(nb::module_& m, const char* name)
{
    using View = urd::index_view<T, Repository>;
    auto cls = nb::class_<View>(m, name)
                   .def("get_index", &View::index)
                   .def("get_source", &View::get_source, nb::keep_alive<0, 1>())
                   .def("get_target", &View::get_target, nb::keep_alive<0, 1>())
                   .def("get_conditions", &View::get_conditions);
    if constexpr (requires { &View::get_effects; })
        cls.def("get_effects", &View::get_effects);
    ygg::add_print(cls);
    urd::python::add_comparison(cls);
    urd::python::add_hash(cls);
    return cls;
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
    using DlRepository = runir::kr::dl::ConstructorRepository;
    using MemoryStateIndex = urd::index<MemoryState>;
    using ConditionIndexList = runir::kr::ps::ConditionList<runir::kr::IcpFamilyTag>;
    using ConditionViewList = urd::Store<urd::sequence<urd::resolved_t<runir::kr::ps::ConditionVariant<runir::kr::IcpFamilyTag>, Repository>>>;
    using EffectIndexList = runir::kr::ps::EffectList<runir::kr::IcpFamilyTag>;
    using EffectViewList = urd::Store<urd::sequence<urd::resolved_t<runir::kr::ps::EffectVariant<runir::kr::IcpFamilyTag>, Repository>>>;
    using ConceptFeatureIndex = urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>;
    using RoleFeatureIndex = urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>>;
    using ConceptRegisterIndex = urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>;
    using RoleRegisterIndex = urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>>;

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
                      urd::index_view<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>, Repository>,
                      urd::index_view<runir::kr::dl::Register<runir::kr::dl::ConceptTag>, DlRepository>,
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
                      urd::index_view<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::RoleTag>, Repository>,
                      urd::index_view<runir::kr::dl::Register<runir::kr::dl::RoleTag>, DlRepository>,
                      const EffectViewList&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "feature"_a,
             "reg"_a,
             "effects"_a)
        .def_rw("feature", &urd::data<RoleLoad>::feature)
        .def_rw("reg", &urd::data<RoleLoad>::reg);
    bind_rule_data<Crule>(m, "CruleRuleData")
        .def(nb::init<MemoryStateIndex,
                      MemoryStateIndex,
                      ConditionIndexList,
                      EffectIndexList,
                      ::cista::offset::string,
                      ::cista::offset::vector<::cista::offset::string>,
                      urd::flat_vector<urd::index<XCondition>>,
                      urd::flat_vector<urd::index<XEffect>>>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "argument_names"_a,
             "xconditions"_a,
             "xeffects"_a)
        .def(urd::python::init_from_views<MemoryStateView,
                      MemoryStateView,
                      const ConditionViewList&,
                      const EffectViewList&,
                      ::cista::offset::string,
                      ::cista::offset::vector<::cista::offset::string>,
                      const urd::Store<urd::sequence<urd::index_view<XCondition, Repository>>>&,
                      const urd::Store<urd::sequence<urd::index_view<XEffect, Repository>>>&>(),
             "source"_a,
             "target"_a,
             "conditions"_a,
             "effects"_a,
             "action_name"_a,
             "argument_names"_a,
             "xconditions"_a,
             "xeffects"_a)
        .def_rw("action_name", &urd::data<Crule>::action_name)
        .def_rw("argument_names", &urd::data<Crule>::argument_names)
        .def_rw("xconditions", &urd::data<Crule>::xconditions)
        .def_rw("xeffects", &urd::data<Crule>::xeffects);
    bind_rule_view<ConceptLoad>(m, "ConceptLoadRule")
        .def("get_feature", &RuleView<LoadTag<runir::kr::dl::ConceptTag>>::get_feature, nb::keep_alive<0, 1>())
        .def("get_register", &RuleView<LoadTag<runir::kr::dl::ConceptTag>>::get_register, nb::keep_alive<0, 1>());
    bind_rule_view<RoleLoad>(m, "RoleLoadRule")
        .def("get_feature", &RuleView<LoadTag<runir::kr::dl::RoleTag>>::get_feature, nb::keep_alive<0, 1>())
        .def("get_register", &RuleView<LoadTag<runir::kr::dl::RoleTag>>::get_register, nb::keep_alive<0, 1>());
    bind_rule_view<Crule>(m, "CruleRule")
        .def("get_action_name", &RuleView<CruleTag>::get_action_name)
        .def("get_argument_names", &RuleView<CruleTag>::get_argument_names)
        .def("get_xconditions", &RuleView<CruleTag>::get_xconditions)
        .def("get_xeffects", &RuleView<CruleTag>::get_xeffects);
    runir::kr::python::bind_insert<ConceptLoad>(repository);
    runir::kr::python::bind_insert<RoleLoad>(repository);
    runir::kr::python::bind_insert<Crule>(repository);
}

}  // namespace runir::kr::ps::icp
