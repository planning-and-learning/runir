#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/ext/dl/feature_data.hpp>
#include <runir/kr/ps/ext/dl/feature_view.hpp>
#include <runir/kr/ps/ext/dl/syntactic_complexity.hpp>
#include <runir/kr/ps/ext/feature_data.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/ext/syntactic_complexity.hpp>
#include <runir/kr/ps/feature_view.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext::dl
{

using namespace nanobind::literals;

namespace
{

template<typename T>
void bind_feature_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<T>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index).def_rw("value", &Data::value);
    ygg::add_comparison(cls);
}

template<typename T>
void bind_concrete_feature_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<T>;
    auto cls = nb::class_<Data>(m, name)
                   .def(nb::init<>())
                   .def_rw("index", &Data::index)
                   .def_rw("feature", &Data::feature)
                   .def_rw("symbol", &Data::symbol);
    ygg::add_comparison(cls);
}

template<typename T>
void bind_feature_view(nb::module_& m, const char* name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    auto cls = nb::class_<View>(m, name)
                   .def("get_index", &View::get_index)
                   .def("get_variant", &View::get_variant)
                   .def("get_expression", &View::get_expression, nb::keep_alive<0, 1>())
                   .def("get_symbol", &View::get_symbol)
                   .def("syntactic_complexity", [](View value) { return runir::kr::ps::ext::syntactic_complexity(value); });
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

template<typename T>
void bind_concrete_feature_view(nb::module_& m, const char* name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    auto cls = nb::class_<View>(m, name)
                   .def("get_index", &View::get_index)
                   .def("get_expression", &View::get_expression, nb::keep_alive<0, 1>())
                   .def("get_symbol", &View::get_symbol)
                   .def("syntactic_complexity", [](View value) { return runir::kr::ps::ext::dl::syntactic_complexity(value); });
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace

void bind_feature(nb::module_& m, RepositoryBinding& repository)
{
    using Concept = runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::ConceptTag>;
    using Role = runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::dl::RoleTag>;
    using Boolean = runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>;
    using Numerical = runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>;
    using ConcreteConcept = runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::ConceptTag>;
    using ConcreteRole = runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::dl::RoleTag>;
    using ConcreteBoolean = runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>;
    using ConcreteNumerical = runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature>;

    ygg::bind_index<ygg::Index<Concept>>(m, "ConceptFeatureIndex");
    ygg::bind_index<ygg::Index<Role>>(m, "RoleFeatureIndex");
    ygg::bind_index<ygg::Index<Boolean>>(m, "BooleanFeatureIndex");
    ygg::bind_index<ygg::Index<Numerical>>(m, "NumericalFeatureIndex");
    ygg::bind_index<ygg::Index<ConcreteConcept>>(m, "ConcreteConceptFeatureIndex");
    ygg::bind_index<ygg::Index<ConcreteRole>>(m, "ConcreteRoleFeatureIndex");
    ygg::bind_index<ygg::Index<ConcreteBoolean>>(m, "ConcreteBooleanFeatureIndex");
    ygg::bind_index<ygg::Index<ConcreteNumerical>>(m, "ConcreteNumericalFeatureIndex");

    bind_feature_data<Concept>(m, "ConceptFeatureData");
    bind_feature_data<Role>(m, "RoleFeatureData");
    bind_feature_data<Boolean>(m, "BooleanFeatureData");
    bind_feature_data<Numerical>(m, "NumericalFeatureData");
    bind_concrete_feature_data<ConcreteConcept>(m, "ConcreteConceptFeatureData");
    bind_concrete_feature_data<ConcreteRole>(m, "ConcreteRoleFeatureData");
    bind_concrete_feature_data<ConcreteBoolean>(m, "ConcreteBooleanFeatureData");
    bind_concrete_feature_data<ConcreteNumerical>(m, "ConcreteNumericalFeatureData");

    bind_feature_view<Concept>(m, "ConceptFeature");
    bind_feature_view<Role>(m, "RoleFeature");
    bind_feature_view<Boolean>(m, "BooleanFeature");
    bind_feature_view<Numerical>(m, "NumericalFeature");
    bind_concrete_feature_view<ConcreteConcept>(m, "ConcreteConceptFeature");
    bind_concrete_feature_view<ConcreteRole>(m, "ConcreteRoleFeature");
    bind_concrete_feature_view<ConcreteBoolean>(m, "ConcreteBooleanFeature");
    bind_concrete_feature_view<ConcreteNumerical>(m, "ConcreteNumericalFeature");

    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Concept, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Role, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Boolean, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Numerical, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<ConcreteConcept, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<ConcreteRole, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<ConcreteBoolean, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<ConcreteNumerical, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::ext::dl
