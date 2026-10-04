#ifndef RUNIR_KR_PS_ICP_REPOSITORY_HPP_
#define RUNIR_KR_PS_ICP_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/icp/canonicalization.hpp"
#include "runir/kr/ps/icp/views.hpp"
#include "runir/kr/ps/repository.hpp"

#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::ps::icp
{

using Builder = ygg::ApplyTypeListT<ygg::formalism::BuilderStorage, RepositoryTypes>;

using ygg::formalism::checkout;

template<typename T>
    requires ygg::formalism::SupportsSymbol<Repository, T>
void prepare_for_interning(Repository&, ygg::Data<T>& data)
{
    canonicalize(data);
}

using ygg::formalism::get_or_create;

}

#endif
