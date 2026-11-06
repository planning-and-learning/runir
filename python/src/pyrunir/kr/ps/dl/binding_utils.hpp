#ifndef PYRUNIR_KR_PS_DL_BINDING_UTILS_HPP_
#define PYRUNIR_KR_PS_DL_BINDING_UTILS_HPP_

#include "pyrunir/kr/binding_utils.hpp"
#include "pyrunir/kr/ps/dl/optimization_bindings.hpp"

#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_storage.hpp>
#include <runir/kr/dl/semantics/register_values_view.hpp>
#include <runir/kr/ps/dl/declarations.hpp>
#include <runir/kr/ps/dl/effect_data.hpp>
#include <runir/kr/ps/family_traits.hpp>
#include <runir/kr/ps/feature_data.hpp>
#include <runir/kr/ps/formatter.hpp>
#include <runir/kr/ps/syntactic_complexity.hpp>
#include <string>
#include <tyr/planning/state_repository.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::python
{

namespace nb = nanobind;

template<typename T, typename Repository>
auto bind_variant(nb::module_& m, nb::class_<Repository>& repository, const std::string& name)
{
    using Data = urd::data<T>;
    using View = urd::view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, name + "Index");
    auto data = nb::class_<Data>(m, (name + "Data").c_str())
                    .def(nb::init<>())
                    .def(nb::init<DataVariant<Data>>(), nb::arg("variant"))
                    .def(urd::python::init_from_views<urd::resolved_t<DataVariant<Data>, Repository>>(), nb::arg("variant"))
                    .def_rw("index", &Data::index)
                    .def_rw("variant", &Data::variant);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, name.c_str()).def("get_index", &View::index).def("get_variant", &View::get_variant);
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    runir::kr::python::bind_insert<T>(repository);
    return view;
}

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename Repository>
auto bind_feature(nb::module_& m, nb::class_<Repository>& repository, const std::string& name)
{
    using Feature = runir::kr::ps::Feature<Family, FeatureTag>;
    using FeatureView = urd::view<Feature, Repository>;
    using ConcreteFeature = runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>;
    using Data = urd::data<ConcreteFeature>;
    using ConcreteView = urd::view<ConcreteFeature, Repository>;

    auto feature = bind_variant<Feature>(m, repository, name);
    feature.def("get_feature", &FeatureView::get_feature, nb::keep_alive<0, 1>())
        .def("get_expression", &FeatureView::get_expression, nb::keep_alive<0, 1>())
        .def("get_symbol", &FeatureView::get_symbol)
        .def("syntactic_complexity", [](FeatureView value) { return runir::kr::ps::syntactic_complexity(value); });
    bind_ps_optimize<Family, Repository>(feature);

    const auto concrete_name = "Concrete" + name;
    urd::python::bind_index<urd::index<ConcreteFeature>>(m, concrete_name + "Index");
    using ExpressionIndex = urd::index<typename Data::Expression>;
    using DlRepository = runir::kr::dl::ConstructorRepository;
    auto data = nb::class_<Data>(m, (concrete_name + "Data").c_str())
                    .def(nb::init<>())
                    .def(nb::init<ExpressionIndex, ::cista::offset::string>(), nb::arg("feature"), nb::arg("symbol"))
                    .def(urd::python::init_from_views<urd::view<typename Data::Expression, DlRepository>, ::cista::offset::string>(), nb::arg("feature"), nb::arg("symbol"))
                    .def_rw("index", &Data::index)
                    .def_rw("feature", &Data::feature)
                    .def_rw("symbol", &Data::symbol);
    urd::python::add_comparison(data);
    auto concrete = nb::class_<ConcreteView>(m, concrete_name.c_str())
                        .def("get_index", &ConcreteView::index)
                        .def("get_feature", &ConcreteView::get_feature, nb::keep_alive<0, 1>())
                        .def("get_expression", &ConcreteView::get_expression, nb::keep_alive<0, 1>())
                        .def("get_symbol", &ConcreteView::get_symbol)
                        .def("syntactic_complexity", [](ConcreteView value) { return runir::kr::ps::dl::syntactic_complexity(value); });
    ygg::add_print(concrete);
    urd::python::add_comparison(concrete);
    urd::python::add_hash(concrete);
    runir::kr::python::bind_insert<ConcreteFeature>(repository);
    return std::pair(feature, concrete);
}

// The feature observed by a concrete condition or effect.
template<typename T>
struct ObservedFeature;

template<typename Family, typename Kind, typename FeatureTag, typename ObservationTag>
struct ObservedFeature<runir::kr::ps::ConcreteCondition<Family, Kind, FeatureTag, ObservationTag>>
{
    using type = runir::kr::ps::Feature<Family, FeatureTag>;
};

template<typename Family, typename Kind, typename FeatureTag, typename ObservationTag>
struct ObservedFeature<runir::kr::ps::ConcreteEffect<Family, Kind, FeatureTag, ObservationTag>>
{
    using type = runir::kr::ps::Feature<Family, FeatureTag>;
};

template<typename T, typename Repository>
auto bind_observation(nb::module_& m, nb::class_<Repository>& repository, const std::string& name)
{
    using Data = urd::data<T>;
    using View = urd::view<T, Repository>;
    urd::python::bind_index<urd::index<T>>(m, name + "Index");
    using FeatureIndex = urd::index<typename ObservedFeature<T>::type>;
    auto data = nb::class_<Data>(m, (name + "Data").c_str())
                    .def(nb::init<>())
                    .def(nb::init<FeatureIndex>(), nb::arg("feature"))
                    .def(urd::python::init_from_views<urd::view<typename ObservedFeature<T>::type, Repository>>(), nb::arg("feature"))
                    .def_rw("index", &Data::index)
                    .def_rw("feature", &Data::feature);
    urd::python::add_comparison(data);
    auto view = nb::class_<View>(m, name.c_str()).def("get_index", &View::index).def("get_feature", &View::get_feature, nb::keep_alive<0, 1>());
    ygg::add_print(view);
    urd::python::add_comparison(view);
    urd::python::add_hash(view);
    runir::kr::python::bind_insert<T>(repository);
    return view;
}

}  // namespace runir::kr::python

#endif
