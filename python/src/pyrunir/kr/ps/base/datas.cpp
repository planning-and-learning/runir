#include "pyrunir/kr/ps/base/module.hpp"

#include <runir/kr/ps/base/repository.hpp>
#include <string>
#include <yggdrasil/python/type_casters.hpp>

namespace runir::kr::ps::base
{
namespace
{

template<typename T>
void bind_data(nb::module_& m, const std::string& name)
{
    using Data = ygg::Data<T>;

    auto cls = nb::class_<Data>(m, name.c_str()).def(nb::init<>());

    if constexpr (requires { &Data::index; })
        cls.def_rw("index", &Data::index);
    if constexpr (requires { &Data::value; })
        cls.def_rw("value", &Data::value);
    if constexpr (requires { &Data::symbol; })
        cls.def_rw("symbol", &Data::symbol);
    if constexpr (requires { &Data::conditions; })
        cls.def_rw("conditions", &Data::conditions);
    if constexpr (requires { &Data::effects; })
        cls.def_rw("effects", &Data::effects);
    if constexpr (requires { &Data::boolean_features; })
        cls.def_rw("boolean_features", &Data::boolean_features);
    if constexpr (requires { &Data::numerical_features; })
        cls.def_rw("numerical_features", &Data::numerical_features);
    if constexpr (requires { &Data::rules; })
        cls.def_rw("rules", &Data::rules);
}

}  // namespace

void bind_datas(nb::module_& m)
{
    bind_data<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>(m, "ConditionVariantData");
    bind_data<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>(m, "EffectVariantData");
    bind_data<Rule>(m, "RuleData");
    bind_data<Sketch>(m, "SketchData");
}

}  // namespace runir::kr::ps::base
