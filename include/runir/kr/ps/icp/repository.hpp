#ifndef RUNIR_KR_PS_ICP_REPOSITORY_HPP_
#define RUNIR_KR_PS_ICP_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/dl/condition_builder.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_builder.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_builder.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_builder.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/icp/canonicalization.hpp"
#include "runir/kr/ps/icp/module_builder.hpp"
#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/program_builder.hpp"
#include "runir/kr/ps/icp/program_view.hpp"
#include "runir/kr/ps/icp/rule_builder.hpp"
#include "runir/kr/ps/icp/rule_view.hpp"
#include "runir/kr/ps/icp/xcondition_builder.hpp"
#include "runir/kr/ps/icp/xcondition_view.hpp"
#include "runir/kr/ps/icp/xeffect_builder.hpp"
#include "runir/kr/ps/icp/xeffect_view.hpp"
#include "runir/kr/ps/memory_state_builder.hpp"
#include "runir/kr/ps/memory_state_view.hpp"
#include "runir/kr/ps/module_symbol_builder.hpp"
#include "runir/kr/ps/module_symbol_view.hpp"
#include "runir/kr/ps/repository.hpp"
#include "runir/kr/ps/rule_variant_builder.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::ps::icp
{

using Builder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, boost::mp11::mp_push_front<RepositoryTypes, Repository>>;

using ygg::formalism::checkout;


}

#endif
