#ifndef PYRUNIR_KR_PS_DL_BINDING_UTILS_HPP_
#define PYRUNIR_KR_PS_DL_BINDING_UTILS_HPP_

#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/dl/repository.hpp>
#include <runir/kr/ps/condition_data.hpp>
#include <runir/kr/ps/dl/declarations.hpp>
#include <runir/kr/ps/dl/effect_data.hpp>
#include <runir/kr/ps/effect_data.hpp>
#include <runir/kr/ps/feature_data.hpp>
#include <runir/kr/ps/formatter.hpp>
#include <runir/kr/ps/syntactic_complexity.hpp>
#include <string>
#include <utility>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::python
{

namespace nb = nanobind;

template<typename T, typename Repository>
auto bind_variant(nb::module_& m, nb::class_<Repository>& repository, const std::string& name)
{
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, (name + "Index").c_str());
    auto data = nb::class_<Data>(m, (name + "Data").c_str()).def(nb::init<>()).def_rw("index", &Data::index).def_rw("variant", &Data::variant);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index).def("get_variant", &View::get_variant);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    repository.def("get_or_create", &get_or_create_data<T, Repository>, nb::arg("data"), nb::keep_alive<0, 1>());
    return view;
}

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename Repository>
auto bind_feature(nb::module_& m, nb::class_<Repository>& repository, const std::string& name)
{
    using Feature = runir::kr::ps::Feature<Family, FeatureTag>;
    using FeatureView = ygg::View<ygg::Index<Feature>, Repository>;
    using ConcreteFeature = runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>;
    using Data = ygg::Data<ConcreteFeature>;
    using ConcreteView = ygg::View<ygg::Index<ConcreteFeature>, Repository>;

    auto feature = bind_variant<Feature>(m, repository, name);
    feature.def("get_feature", &FeatureView::get_feature, nb::keep_alive<0, 1>())
        .def("get_expression", &FeatureView::get_expression, nb::keep_alive<0, 1>())
        .def("get_symbol", &FeatureView::get_symbol)
        .def("syntactic_complexity", [](FeatureView value) { return runir::kr::ps::syntactic_complexity(value); });

    const auto concrete_name = "Concrete" + name;
    ygg::bind_index<ygg::Index<ConcreteFeature>>(m, (concrete_name + "Index").c_str());
    auto data = nb::class_<Data>(m, (concrete_name + "Data").c_str())
                    .def(nb::init<>())
                    .def_rw("index", &Data::index)
                    .def_rw("feature", &Data::feature)
                    .def_rw("symbol", &Data::symbol);
    ygg::add_comparison(data);
    auto concrete = nb::class_<ConcreteView>(m, concrete_name.c_str())
                        .def("get_index", &ConcreteView::get_index)
                        .def("get_feature", &ConcreteView::get_feature, nb::keep_alive<0, 1>())
                        .def("get_expression", &ConcreteView::get_expression, nb::keep_alive<0, 1>())
                        .def("get_symbol", &ConcreteView::get_symbol)
                        .def("syntactic_complexity", [](ConcreteView value) { return runir::kr::ps::dl::syntactic_complexity(value); });
    ygg::add_print(concrete);
    ygg::add_comparison(concrete);
    ygg::add_hash(concrete);
    repository.def("get_or_create", &get_or_create_data<ConcreteFeature, Repository>, nb::arg("data"), nb::keep_alive<0, 1>());
    return std::pair(feature, concrete);
}

template<typename T, typename Repository>
auto bind_observation(nb::module_& m, nb::class_<Repository>& repository, const std::string& name)
{
    using Data = ygg::Data<T>;
    using View = ygg::View<ygg::Index<T>, Repository>;
    ygg::bind_index<ygg::Index<T>>(m, (name + "Index").c_str());
    auto data = nb::class_<Data>(m, (name + "Data").c_str()).def(nb::init<>()).def_rw("index", &Data::index).def_rw("feature", &Data::feature);
    ygg::add_comparison(data);
    auto view = nb::class_<View>(m, name.c_str()).def("get_index", &View::get_index).def("get_feature", &View::get_feature, nb::keep_alive<0, 1>());
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    repository.def("get_or_create", &get_or_create_data<T, Repository>, nb::arg("data"), nb::keep_alive<0, 1>());
    return view;
}

}  // namespace runir::kr::python

#endif
