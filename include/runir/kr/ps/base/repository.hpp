#ifndef RUNIR_KR_PS_BASE_REPOSITORY_HPP_
#define RUNIR_KR_PS_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/base/canonicalization.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_builder.hpp"
#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_builder.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/dl/condition_builder.hpp"
#include "runir/kr/ps/dl/condition_data.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_builder.hpp"
#include "runir/kr/ps/dl/effect_data.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_builder.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_builder.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/memory_state_builder.hpp"
#include "runir/kr/ps/module_symbol_builder.hpp"
#include "runir/kr/ps/repository.hpp"
#include "runir/kr/ps/rule_variant_builder.hpp"

#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::ps::base
{

using Builder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, boost::mp11::mp_push_front<RepositoryTypes, Repository>>;

using ygg::formalism::checkout;

}

// Its symbols' reads, finds and inserts instantiated once, where the library instantiates the repository (brokkr's
// include rule I7): a file that reads records through it completes none of its stores.
YGG_FORMALISM_SYMBOL_CONTEXT(extern, runir::kr::ps::base::Repository, 16)

#endif
