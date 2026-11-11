#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/xcondition_view.hpp>
#include <runir/kr/ps/icp/xeffect_view.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::icp
{

namespace
{

template<typename T, typename Operation>
void bind_indexical_type(nb::module_& m, RepositoryBinding& repository, const char* name)
{
    using namespace nb::literals;
    using Data = urd::data<T>;
    using View = urd::index_view<T, Repository>;
    using ObjectReference = urd::flat_variant<ArgumentPosition, urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>>;
    using FeatureIndex = urd::index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>>;
    urd::python::bind_index<urd::index<T>>(m, std::string(name) + "Index");
    auto data =
        nb::class_<Data>(m, (std::string(name) + "Data").c_str())
            .def(nb::init<>())
            .def(nb::init<Operation, ObjectReference, FeatureIndex>(), "operation"_a, "object"_a, "feature"_a)
            .def(urd::python::init_from_views<Operation,
                                              ObjectReference,
                                              urd::index_view<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>, Repository>>(),
                 "operation"_a,
                 "object"_a,
                 "feature"_a)
            .def_rw("index", &Data::index)
            .def_rw("operation", &Data::operation)
            .def_rw("object", &Data::object)
            .def_rw("feature", &Data::feature);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, name)
                    .def("index", &View::index)
                    .def("get_operation", &View::get_operation)
                    .def("get_object_reference", &View::get_object_reference)
                    .def("get_concept_feature", &View::get_concept_feature);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    ygg::bind_insert<T>(repository);
}

}

void bind_indexical(nb::module_& m, RepositoryBinding& repository)
{
    nb::enum_<ConditionOperation>(m, "ConditionOperation").value("BELONGS", ConditionOperation::BELONGS).value("NOT_BELONGS", ConditionOperation::NOT_BELONGS);
    nb::enum_<EffectOperation>(m, "EffectOperation").value("ENTER", EffectOperation::ENTER).value("EXIT", EffectOperation::EXIT);
    ygg::bind_unsigned_integer<ArgumentPosition>(m, "ArgumentPosition");
    auto reset_pair = nb::class_<ResetPair>(m, "ResetPair").def(nb::init<>()).def_rw("before", &ResetPair::before).def_rw("after", &ResetPair::after);
    reset_pair.def(
                  "__eq__",
                  [](const ResetPair& lhs, const ResetPair& rhs) { return lhs == rhs; },
                  nb::is_operator())
        .def("__ne__", [](const ResetPair& lhs, const ResetPair& rhs) { return lhs != rhs; }, nb::is_operator());
    bind_indexical_type<XCondition, ConditionOperation>(m, repository, "XCondition");
    bind_indexical_type<XEffect, EffectOperation>(m, repository, "XEffect");
}

}
