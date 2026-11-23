#include "bindings.hpp"
#include "pyrunir/kr/dl/conversions.hpp"
#include "pyrunir/kr/ps/icp/conversions.hpp"
#include "pyrunir/module.hpp"

#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/reset_pair_builder.hpp>
#include <runir/kr/ps/icp/reset_pair_view.hpp>
#include <runir/kr/ps/icp/xcondition_view.hpp>
#include <runir/kr/ps/icp/xeffect_view.hpp>
#include <urd/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

namespace runir::kr::ps::icp
{

void bind_indexical(nb::module_& m, RepositoryBinding& repository)
{
    nb::enum_<ConditionOperation>(m, "ConditionOperation").value("BELONGS", ConditionOperation::BELONGS).value("NOT_BELONGS", ConditionOperation::NOT_BELONGS);
    nb::enum_<EffectOperation>(m, "EffectOperation").value("ENTER", EffectOperation::ENTER).value("EXIT", EffectOperation::EXIT);
    ygg::bind_unsigned_integer<ArgumentPosition>(m, "ArgumentPosition");
    urd::python::bind_data<ResetPair>(m);
    urd::python::bind_builder<ResetPair, Repository>(m);
    urd::python::bind_view<ResetPairView>(m);
    urd::python::bind_index<XCondition>(m);
    urd::python::bind_data<XCondition>(m);
    urd::python::bind_index<XEffect>(m);
    urd::python::bind_data<XEffect>(m);
    urd::python::bind_view<urd::index_view<XCondition, Repository>>(m);
    urd::python::bind_builder<XCondition, Repository>(m);
    urd::python::bind_view<urd::index_view<XEffect, Repository>>(m);
    urd::python::bind_builder<XEffect, Repository>(m);
    ygg::bind_insert<XCondition>(repository);
    ygg::bind_insert<XEffect>(repository);
}

}
