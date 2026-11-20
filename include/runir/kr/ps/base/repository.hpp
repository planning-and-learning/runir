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

namespace runir::kr::ps::base
{

using Builder = boost::mp11::mp_apply<ygg::formalism::BuilderStorage, boost::mp11::mp_push_front<RepositoryTypes, Repository>>;

using ygg::formalism::checkout;

template<typename T>
    requires ygg::formalism::SupportsSymbol<Repository, T>
void prepare_for_insert(Repository&, urd::data<T>& data)
{ canonicalize(data); }

using ygg::formalism::insert;
}

#endif
