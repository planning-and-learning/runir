#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <nanobind/stl/list.h>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/rule_data.hpp>
#include <runir/kr/ps/ext/rule_view.hpp>
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
                   .def_rw("target", &Data::target)
                   .def_rw("conditions", &Data::conditions);
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
                   .def("get_target", &View::get_target, nb::keep_alive<0, 1>())
                   .def("get_conditions", &View::get_conditions);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
    return cls;
}

}  // namespace

void bind_rule(nb::module_& m, RepositoryBinding& repository)
{
    using ConceptLoad = Rule<LoadTag<runir::kr::dl::ConceptTag>>;
    using RoleLoad = Rule<LoadTag<runir::kr::dl::RoleTag>>;
    using Sketch = Rule<SketchTag>;
    using Do = Rule<DoTag>;
    using Call = Rule<CallTag>;

    ygg::bind_index<ygg::Index<ConceptLoad>>(m, "ConceptLoadRuleIndex");
    ygg::bind_index<ygg::Index<RoleLoad>>(m, "RoleLoadRuleIndex");
    ygg::bind_index<ygg::Index<Sketch>>(m, "SketchRuleIndex");
    ygg::bind_index<ygg::Index<Do>>(m, "DoRuleIndex");
    ygg::bind_index<ygg::Index<Call>>(m, "CallRuleIndex");

    bind_rule_data<ConceptLoad>(m, "ConceptLoadRuleData").def_rw("feature", &ygg::Data<ConceptLoad>::feature).def_rw("reg", &ygg::Data<ConceptLoad>::reg);
    bind_rule_data<RoleLoad>(m, "RoleLoadRuleData").def_rw("feature", &ygg::Data<RoleLoad>::feature).def_rw("reg", &ygg::Data<RoleLoad>::reg);
    bind_rule_data<Sketch>(m, "SketchRuleData").def_rw("effects", &ygg::Data<Sketch>::effects);
    bind_rule_data<Do>(m, "DoRuleData")
        .def_rw("effects", &ygg::Data<Do>::effects)
        .def_rw("action_name", &ygg::Data<Do>::action_name)
        .def_rw("arguments", &ygg::Data<Do>::arguments);
    bind_rule_data<Call>(m, "CallRuleData").def_rw("callee", &ygg::Data<Call>::callee).def_rw("arguments", &ygg::Data<Call>::arguments);

    bind_rule_view<ConceptLoad>(m, "ConceptLoadRule")
        .def("get_feature", &ygg::View<ygg::Index<ConceptLoad>, Repository>::get_feature, nb::keep_alive<0, 1>())
        .def("get_register", &ygg::View<ygg::Index<ConceptLoad>, Repository>::get_register, nb::keep_alive<0, 1>());
    bind_rule_view<RoleLoad>(m, "RoleLoadRule")
        .def("get_feature", &ygg::View<ygg::Index<RoleLoad>, Repository>::get_feature, nb::keep_alive<0, 1>())
        .def("get_register", &ygg::View<ygg::Index<RoleLoad>, Repository>::get_register, nb::keep_alive<0, 1>());
    bind_rule_view<Sketch>(m, "SketchRule").def("get_effects", &ygg::View<ygg::Index<Sketch>, Repository>::get_effects);
    bind_rule_view<Do>(m, "DoRule")
        .def("get_effects", &ygg::View<ygg::Index<Do>, Repository>::get_effects)
        .def("get_action_name", &ygg::View<ygg::Index<Do>, Repository>::get_action_name)
        .def("get_action_arguments", &ygg::View<ygg::Index<Do>, Repository>::get_action_arguments);
    using CallView = ygg::View<ygg::Index<Call>, Repository>;
    bind_rule_view<Call>(m, "CallRule")
        .def("get_callee", &CallView::get_callee, nb::keep_alive<0, 1>())
        .def(
            "get_call_arguments",
            [](const CallView& view)
            {
                nb::list result;
                view.for_each_call_argument([&](auto argument) { result.append(nb::cast(argument)); });
                return result;
            },
            nb::keep_alive<0, 1>());

    repository.def("get_or_create", &runir::kr::python::get_or_create_data<ConceptLoad, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<RoleLoad, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Sketch, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Do, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Call, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::ext
