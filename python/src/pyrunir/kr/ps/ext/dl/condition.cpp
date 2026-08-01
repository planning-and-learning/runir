#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/condition_view.hpp>
#include <runir/kr/ps/ext/dl/condition_data.hpp>
#include <runir/kr/ps/ext/dl/condition_view.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <yggdrasil/python/bindings.hpp>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::ext::dl
{

using namespace nanobind::literals;

namespace
{

template<typename T>
void bind_condition_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<T>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index).def_rw("feature", &Data::feature);
    ygg::add_comparison(cls);
}

template<typename T>
void bind_condition_view(nb::module_& m, const char* name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace

void bind_condition(nb::module_& m, RepositoryBinding& repository)
{
    using Variant = runir::kr::ps::ConcreteConditionVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>;
    using PositiveBoolean = runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>;
    using NegativeBoolean = runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>;
    using EqualZero = runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::EqualZero>;
    using GreaterZero = runir::kr::ps::ConcreteCondition<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::GreaterZero>;
    using VariantData = ygg::Data<Variant>;
    using VariantView = ygg::View<ygg::Index<Variant>, Repository>;

    ygg::bind_index<ygg::Index<Variant>>(m, "ConcreteConditionVariantIndex");
    ygg::bind_index<ygg::Index<PositiveBoolean>>(m, "PositiveBooleanConditionIndex");
    ygg::bind_index<ygg::Index<NegativeBoolean>>(m, "NegativeBooleanConditionIndex");
    ygg::bind_index<ygg::Index<EqualZero>>(m, "EqualZeroNumericalConditionIndex");
    ygg::bind_index<ygg::Index<GreaterZero>>(m, "GreaterZeroNumericalConditionIndex");

    auto data = nb::class_<VariantData>(m, "ConcreteConditionVariantData")
                    .def(nb::init<>())
                    .def_rw("index", &VariantData::index)
                    .def_rw("value", &VariantData::value);
    ygg::add_comparison(data);
    bind_condition_data<PositiveBoolean>(m, "PositiveBooleanConditionData");
    bind_condition_data<NegativeBoolean>(m, "NegativeBooleanConditionData");
    bind_condition_data<EqualZero>(m, "EqualZeroNumericalConditionData");
    bind_condition_data<GreaterZero>(m, "GreaterZeroNumericalConditionData");

    auto view = nb::class_<VariantView>(m, "ConcreteConditionVariant").def("get_index", &VariantView::get_index).def("get_variant", &VariantView::get_variant);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    bind_condition_view<PositiveBoolean>(m, "PositiveBooleanCondition");
    bind_condition_view<NegativeBoolean>(m, "NegativeBooleanCondition");
    bind_condition_view<EqualZero>(m, "EqualZeroNumericalCondition");
    bind_condition_view<GreaterZero>(m, "GreaterZeroNumericalCondition");

    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Variant, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<PositiveBoolean, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<NegativeBoolean, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<EqualZero, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<GreaterZero, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::ext::dl
