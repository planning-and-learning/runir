#ifndef RUNIR_KR_PS_BASE_REPOSITORY_HPP_
#define RUNIR_KR_PS_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/base/canonicalization.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/rule_data.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_data.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/condition_data.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/dl/condition_data.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_data.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/effect_data.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/repository.hpp"

#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::ps::base
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
