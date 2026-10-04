#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <nanobind/stl/list.h>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/rule_data.hpp>
#include <runir/kr/ps/icp/rule_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::icp
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
    if constexpr (requires { &Data::effects; })
        cls.def_rw("effects", &Data::effects);
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
    if constexpr (requires { &View::get_effects; })
        cls.def("get_effects", &View::get_effects);
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
    using Crule = Rule<CruleTag>;

    ygg::bind_index<ygg::Index<ConceptLoad>>(m, "ConceptLoadRuleIndex");
    ygg::bind_index<ygg::Index<RoleLoad>>(m, "RoleLoadRuleIndex");
    ygg::bind_index<ygg::Index<Crule>>(m, "CruleRuleIndex");
    bind_rule_data<ConceptLoad>(m, "ConceptLoadRuleData").def_rw("feature", &ygg::Data<ConceptLoad>::feature).def_rw("reg", &ygg::Data<ConceptLoad>::reg);
    bind_rule_data<RoleLoad>(m, "RoleLoadRuleData").def_rw("feature", &ygg::Data<RoleLoad>::feature).def_rw("reg", &ygg::Data<RoleLoad>::reg);
    bind_rule_data<Crule>(m, "CruleRuleData")
        .def_rw("action_name", &ygg::Data<Crule>::action_name)
        .def_rw("argument_names", &ygg::Data<Crule>::argument_names)
        .def_rw("xconditions", &ygg::Data<Crule>::xconditions)
        .def_rw("xeffects", &ygg::Data<Crule>::xeffects);
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
