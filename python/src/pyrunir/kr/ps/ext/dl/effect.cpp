#include "bindings.hpp"
#include "pyrunir/kr/binding_utils.hpp"

#include <runir/kr/ps/effect_view.hpp>
#include <runir/kr/ps/ext/dl/effect_data.hpp>
#include <runir/kr/ps/ext/dl/effect_view.hpp>
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
void bind_effect_data(nb::module_& m, const char* name)
{
    using Data = ygg::Data<T>;
    auto cls = nb::class_<Data>(m, name).def(nb::init<>()).def_rw("index", &Data::index).def_rw("feature", &Data::feature);
    ygg::add_comparison(cls);
}

template<typename T>
void bind_effect_view(nb::module_& m, const char* name)
{
    using View = ygg::View<ygg::Index<T>, Repository>;
    auto cls = nb::class_<View>(m, name).def("get_index", &View::get_index);
    ygg::add_print(cls);
    ygg::add_comparison(cls);
    ygg::add_hash(cls);
}

}  // namespace

void bind_effect(nb::module_& m, RepositoryBinding& repository)
{
    using Variant = runir::kr::ps::ConcreteEffectVariant<runir::kr::ExtFamilyTag, runir::kr::DlTag>;
    using PositiveBoolean = runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Positive>;
    using NegativeBoolean = runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Negative>;
    using UnchangedBoolean = runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature, runir::kr::ps::dl::Unchanged>;
    using Increases = runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Increases>;
    using Decreases = runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Decreases>;
    using UnchangedNumerical = runir::kr::ps::ConcreteEffect<runir::kr::ExtFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::NumericalFeature, runir::kr::ps::dl::Unchanged>;
    using VariantData = ygg::Data<Variant>;
    using VariantView = ygg::View<ygg::Index<Variant>, Repository>;

    ygg::bind_index<ygg::Index<Variant>>(m, "ConcreteEffectVariantIndex");
    ygg::bind_index<ygg::Index<PositiveBoolean>>(m, "PositiveBooleanEffectIndex");
    ygg::bind_index<ygg::Index<NegativeBoolean>>(m, "NegativeBooleanEffectIndex");
    ygg::bind_index<ygg::Index<UnchangedBoolean>>(m, "UnchangedBooleanEffectIndex");
    ygg::bind_index<ygg::Index<Increases>>(m, "IncreasesNumericalEffectIndex");
    ygg::bind_index<ygg::Index<Decreases>>(m, "DecreasesNumericalEffectIndex");
    ygg::bind_index<ygg::Index<UnchangedNumerical>>(m, "UnchangedNumericalEffectIndex");

    auto data = nb::class_<VariantData>(m, "ConcreteEffectVariantData")
                    .def(nb::init<>())
                    .def_rw("index", &VariantData::index)
                    .def_rw("value", &VariantData::value);
    ygg::add_comparison(data);
    bind_effect_data<PositiveBoolean>(m, "PositiveBooleanEffectData");
    bind_effect_data<NegativeBoolean>(m, "NegativeBooleanEffectData");
    bind_effect_data<UnchangedBoolean>(m, "UnchangedBooleanEffectData");
    bind_effect_data<Increases>(m, "IncreasesNumericalEffectData");
    bind_effect_data<Decreases>(m, "DecreasesNumericalEffectData");
    bind_effect_data<UnchangedNumerical>(m, "UnchangedNumericalEffectData");

    auto view = nb::class_<VariantView>(m, "ConcreteEffectVariant").def("get_index", &VariantView::get_index).def("get_variant", &VariantView::get_variant);
    ygg::add_print(view);
    ygg::add_comparison(view);
    ygg::add_hash(view);
    bind_effect_view<PositiveBoolean>(m, "PositiveBooleanEffect");
    bind_effect_view<NegativeBoolean>(m, "NegativeBooleanEffect");
    bind_effect_view<UnchangedBoolean>(m, "UnchangedBooleanEffect");
    bind_effect_view<Increases>(m, "IncreasesNumericalEffect");
    bind_effect_view<Decreases>(m, "DecreasesNumericalEffect");
    bind_effect_view<UnchangedNumerical>(m, "UnchangedNumericalEffect");

    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Variant, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<PositiveBoolean, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<NegativeBoolean, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<UnchangedBoolean, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Increases, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<Decreases, Repository>, "data"_a, nb::keep_alive<0, 1>());
    repository.def("get_or_create", &runir::kr::python::get_or_create_data<UnchangedNumerical, Repository>, "data"_a, nb::keep_alive<0, 1>());
}

}  // namespace runir::kr::ps::ext::dl
