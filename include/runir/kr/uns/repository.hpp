#ifndef RUNIR_KR_UNS_REPOSITORY_HPP_
#define RUNIR_KR_UNS_REPOSITORY_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/ps/dl/feature_data.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/repository.hpp"
#include "runir/kr/uns/canonicalization.hpp"
#include "runir/kr/uns/classifier_data.hpp"
#include "runir/kr/uns/classifier_view.hpp"
#include "runir/kr/uns/declarations.hpp"

#include <yggdrasil/formalism/builder.hpp>
#include <yggdrasil/formalism/interning.hpp>

namespace runir::kr::uns
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
