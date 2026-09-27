#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/xcondition_view.hpp>
#include <runir/kr/ps/icp/xeffect_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::icp
{

using namespace nanobind::literals;

namespace
{

template<typename T>
void bind_indexical_type(nb::module_& m, RepositoryBinding& repository, const char* name)
{
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, (std::string(name) + "Index").c_str());
    auto data = nb::class_<Data>(m, (std::string(name) + "Data").c_str())
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("operation", &Data::operation)
                    .def_rw("object", &Data::object)
                    .def_rw("feature", &Data::feature);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, name)
                    .def("get_index", &View::get_index)
                    .def("get_operation", &View::get_operation)
                    .def("get_object_reference", &View::get_object_reference)
                    .def("get_concept_feature", &View::get_concept_feature, nb::keep_alive<0, 1>());
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<T, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}

void bind_indexical(nb::module_& m, RepositoryBinding& repository)
{
    nb::enum_<ConditionOperation>(m, "ConditionOperation").value("BELONGS", ConditionOperation::BELONGS).value("NOT_BELONGS", ConditionOperation::NOT_BELONGS);
    nb::enum_<EffectOperation>(m, "EffectOperation").value("ENTER", EffectOperation::ENTER).value("EXIT", EffectOperation::EXIT);
    ygg::bind_fixed_uint<ArgumentPosition>(m, "ArgumentPosition");
    auto reset_pair = nb::class_<ResetPair>(m, "ResetPair").def(nb::init<>()).def_rw("before", &ResetPair::before).def_rw("after", &ResetPair::after);
    reset_pair.def(nb::self == nb::self).def(nb::self != nb::self);
    bind_indexical_type<XCondition>(m, repository, "XCondition");
    bind_indexical_type<XEffect>(m, repository, "XEffect");
}

}
