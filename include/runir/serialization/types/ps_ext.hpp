#ifndef RUNIR_SERIALIZATION_TYPES_PS_EXT_HPP_
#define RUNIR_SERIALIZATION_TYPES_PS_EXT_HPP_

#include "runir/kr/ps/ext/repository.hpp"
#include "runir/serialization/types/views.hpp"

namespace runir::serialization
{

using ExtTypes =
    boost::mp11::mp_append<kr::ps::ext::FeatureTypes, kr::ps::ext::ConditionTypes, kr::ps::ext::EffectTypes, kr::ps::ext::RuleTypes, kr::ps::ext::ProgramTypes>;
using ExtViews = boost::mp11::mp_transform_q<boost::mp11::mp_bind_front<IndexView, kr::ps::ext::Repository>, ExtTypes>;

}  // namespace runir::serialization

#endif
