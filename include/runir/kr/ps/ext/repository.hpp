#ifndef RUNIR_KR_PS_EXT_REPOSITORY_HPP_
#define RUNIR_KR_PS_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/condition_data.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/effect_data.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/ext/canonicalization.hpp"
#include "runir/kr/ps/ext/memory_state_data.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/module_symbol_data.hpp"
#include "runir/kr/ps/ext/module_symbol_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/program_data.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_variant_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/feature_data.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::ps::ext
{

using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, RepositoryTypes>;

using ygg::formalism::checkout;

template<typename T>
    requires ygg::formalism::SupportsSymbol<Repository, T>
void prepare_for_insert(Repository&, ygg::Data<T>& data)
{
    canonicalize(data);
}

using ygg::formalism::insert;
}

#endif
